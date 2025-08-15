#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDir>

class HeadlessOperationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure we can find the executable
        executablePath = QDir::current().absoluteFilePath("stdgeo_viewer");
        if (!QFile::exists(executablePath)) {
            // Try relative path from build directory
            executablePath = QDir::current().absoluteFilePath("./stdgeo_viewer");
        }
        ASSERT_TRUE(QFile::exists(executablePath)) << "stdgeo_viewer executable not found at: " << executablePath.toStdString();
    }
    
    // Helper function to run headless command and get output
    QString runHeadlessCommand(const QString& command) {
        QProcess process;
        process.setProgram(executablePath);
        process.setArguments({"--headless"});
        
        process.start();
        EXPECT_TRUE(process.waitForStarted(5000)) << "Failed to start headless process";
        
        // Send command
        process.write((command + "\nexit\n").toUtf8());
        process.closeWriteChannel();
        
        EXPECT_TRUE(process.waitForFinished(10000)) << "Process did not finish in time";
        EXPECT_EQ(process.exitCode(), 0) << "Process exited with error code: " << process.exitCode();
        
        return QString::fromUtf8(process.readAllStandardOutput());
    }
    
    // Helper function to run script file
    QString runHeadlessScript(const QString& scriptContent) {
        QTemporaryFile tempFile;
        EXPECT_TRUE(tempFile.open()) << "Could not create temporary file";
        
        QTextStream stream(&tempFile);
        stream << scriptContent;
        tempFile.close();
        
        QProcess process;
        process.setProgram("/bin/bash");
        process.setArguments({"-c", QString("cat %1 | %2 --headless").arg(tempFile.fileName()).arg(executablePath)});
        
        process.start();
        EXPECT_TRUE(process.waitForStarted(5000)) << "Failed to start script process";
        EXPECT_TRUE(process.waitForFinished(15000)) << "Script process did not finish in time";
        
        return QString::fromUtf8(process.readAllStandardOutput());
    }
    
    QString executablePath;
};

// Basic command tests
TEST_F(HeadlessOperationTest, HelpCommand) {
    QString output = runHeadlessCommand("help");
    
    EXPECT_TRUE(output.contains("Available Commands")) << "Help output missing header";
    EXPECT_TRUE(output.contains("cube")) << "Help missing cube command";
    EXPECT_TRUE(output.contains("clear")) << "Help missing clear command";
    EXPECT_TRUE(output.contains("stats")) << "Help missing stats command";
    EXPECT_TRUE(output.contains("exit")) << "Help missing exit command";
}

TEST_F(HeadlessOperationTest, BasicCubeCreation) {
    QString output = runHeadlessCommand("cube 2.0");
    
    EXPECT_TRUE(output.contains("Created cube")) << "Cube creation message not found";
    EXPECT_TRUE(output.contains("size: 2")) << "Cube size not reported correctly";
    EXPECT_TRUE(output.contains("8 vertices")) << "Incorrect vertex count";
    EXPECT_TRUE(output.contains("12 triangles")) << "Incorrect triangle count";
}

TEST_F(HeadlessOperationTest, CubeStatistics) {
    QString output = runHeadlessScript("cube 3.0\nstats");
    
    EXPECT_TRUE(output.contains("Current geometry: 8 vertices, 12 triangles")) << "Stats format incorrect";
    EXPECT_TRUE(output.contains("Sample vertices:")) << "Sample vertices not shown";
    EXPECT_TRUE(output.contains("(-1.5, -1.5, -1.5)")) << "Vertex coordinates incorrect for size 3.0";
}

TEST_F(HeadlessOperationTest, ClearOperation) {
    QString output = runHeadlessScript("cube 1.0\nclear\nstats");
    
    EXPECT_TRUE(output.contains("Cleared geometry")) << "Clear message not found";
    EXPECT_TRUE(output.contains("No geometry loaded")) << "Stats after clear incorrect";
}

TEST_F(HeadlessOperationTest, MultipleCubes) {
    QString output = runHeadlessScript("cube 1.0\ncube 2.0\nstats");
    
    // Should replace previous cube with new one
    EXPECT_TRUE(output.contains("Created cube (size: 1)")) << "First cube creation not found";
    EXPECT_TRUE(output.contains("Created cube (size: 2)")) << "Second cube creation not found";
    EXPECT_TRUE(output.contains("Current geometry: 8 vertices, 12 triangles")) << "Final stats incorrect";
}

// Edge case tests
TEST_F(HeadlessOperationTest, InvalidCommands) {
    QString output = runHeadlessCommand("invalid_command");
    
    EXPECT_TRUE(output.contains("Unknown command")) << "Invalid command not handled properly";
    EXPECT_TRUE(output.contains("Type 'help'")) << "Help suggestion not provided";
}

TEST_F(HeadlessOperationTest, InvalidCubeSize) {
    QString output = runHeadlessScript("cube invalid_size\ncube -1.0\ncube abc");
    
    EXPECT_TRUE(output.contains("Error: Invalid cube size") || 
                output.contains("Unknown command")) << "Invalid cube parameters not handled";
}

TEST_F(HeadlessOperationTest, EdgeCaseSizes) {
    QString output = runHeadlessScript("cube 0.1\nstats\ncube 100.0\nstats");
    
    EXPECT_TRUE(output.contains("Created cube (size: 0.1)")) << "Small cube not created";
    EXPECT_TRUE(output.contains("Created cube (size: 100)")) << "Large cube not created";
    EXPECT_TRUE(output.contains("8 vertices")) << "Vertex count wrong for edge cases";
}

TEST_F(HeadlessOperationTest, ZeroSizeCube) {
    QString output = runHeadlessCommand("cube 0.0");
    
    // Should either create cube with 0 size or handle gracefully
    EXPECT_TRUE(output.contains("Created cube") || output.contains("Error")) << "Zero size not handled";
}

// Workflow tests
TEST_F(HeadlessOperationTest, BasicWorkflow) {
    QString script = R"(
        help
        cube 2.0
        stats
        clear
        cube 1.0
        stats
        clear
    )";
    
    QString output = runHeadlessScript(script);
    
    EXPECT_TRUE(output.contains("Available Commands")) << "Help not shown";
    EXPECT_TRUE(output.contains("Created cube (size: 2)")) << "First cube not created";
    EXPECT_TRUE(output.contains("Created cube (size: 1)")) << "Second cube not created";
    EXPECT_TRUE(output.contains("Cleared geometry")) << "Clear operations not working";
}

TEST_F(HeadlessOperationTest, CommentHandling) {
    QString script = R"(
        # This is a comment
        cube 1.0
        # Another comment
        stats
        # Final comment
    )";
    
    QString output = runHeadlessScript(script);
    
    EXPECT_TRUE(output.contains("Created cube")) << "Commands after comments not executed";
    EXPECT_TRUE(output.contains("Current geometry")) << "Stats after comments not working";
    EXPECT_FALSE(output.contains("Unknown command: #")) << "Comments processed as commands";
}

// Performance and stress tests
TEST_F(HeadlessOperationTest, RapidOperations) {
    QString script = R"(
        cube 1.0
        clear
        cube 2.0
        clear
        cube 3.0
        clear
        cube 4.0
        stats
    )";
    
    QString output = runHeadlessScript(script);
    
    EXPECT_TRUE(output.contains("Current geometry: 8 vertices, 12 triangles")) << "Rapid operations failed";
}

TEST_F(HeadlessOperationTest, LargeCube) {
    QString output = runHeadlessCommand("cube 1000.0");
    
    EXPECT_TRUE(output.contains("Created cube (size: 1000)")) << "Large cube creation failed";
    EXPECT_TRUE(output.contains("8 vertices")) << "Large cube has wrong vertex count";
}

TEST_F(HeadlessOperationTest, PrecisionTest) {
    QString output = runHeadlessCommand("cube 3.14159");
    
    EXPECT_TRUE(output.contains("Created cube (size: 3.14159)")) << "Precision cube creation failed";
}

// Example script validation tests
TEST_F(HeadlessOperationTest, BasicOperationsExample) {
    // Test that our basic_operations.txt example works
    QString examplePath = QDir::current().absoluteFilePath("../examples/basic_operations.txt");
    if (!QFile::exists(examplePath)) {
        GTEST_SKIP() << "Example file not found: " << examplePath.toStdString();
    }
    
    QProcess process;
    process.setProgram("/bin/bash");
    process.setArguments({"-c", QString("cat %1 | %2 --headless").arg(examplePath).arg(executablePath)});
    
    process.start();
    ASSERT_TRUE(process.waitForStarted(5000));
    ASSERT_TRUE(process.waitForFinished(15000));
    EXPECT_EQ(process.exitCode(), 0) << "Basic operations example failed";
    
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    EXPECT_TRUE(output.contains("Created cube")) << "Example did not create cubes";
}

TEST_F(HeadlessOperationTest, ErrorHandlingExample) {
    // Test error handling robustness
    QString examplePath = QDir::current().absoluteFilePath("../examples/error_handling.txt");
    if (!QFile::exists(examplePath)) {
        GTEST_SKIP() << "Error handling example not found";
    }
    
    QProcess process;
    process.setProgram("/bin/bash");
    process.setArguments({"-c", QString("cat %1 | %2 --headless").arg(examplePath).arg(executablePath)});
    
    process.start();
    ASSERT_TRUE(process.waitForStarted(5000));
    ASSERT_TRUE(process.waitForFinished(15000));
    
    // Should exit normally even with errors
    EXPECT_EQ(process.exitCode(), 0) << "Error handling example should not crash";
}