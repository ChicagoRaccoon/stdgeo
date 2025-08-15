#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QMouseEvent>
#include <QWheelEvent>
#include "../src/qt/geometryviewer.h"
#include "../src/qt/mainwindow.h"
#include "../src/qt/commandline.h"

class GeometryViewerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 1;
            static char arg0[] = "stdgeo_tests";
            static char* argv[] = {arg0, nullptr};
            app = new QApplication(argc, argv);
        }
        viewer = new GeometryViewer();
    }
    
    void TearDown() override {
        delete viewer;
        // Don't delete app as it's shared
    }
    
    GeometryViewer* viewer;
    static QApplication* app;
};

QApplication* GeometryViewerTest::app = nullptr;

TEST_F(GeometryViewerTest, InitialState) {
    EXPECT_NE(viewer, nullptr);
    // Viewer should be created without errors
}

TEST_F(GeometryViewerTest, CreateCube) {
    // Test cube creation
    viewer->createCube(2.0);
    
    // We can't easily test the internal mesh state without exposing it,
    // but we can verify the call doesn't crash
    SUCCEED();
}

TEST_F(GeometryViewerTest, ClearMesh) {
    viewer->createCube(2.0);
    viewer->clearMesh();
    
    // Should not crash
    SUCCEED();
}

class CommandLineTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 1;
            static char arg0[] = "stdgeo_tests";
            static char* argv[] = {arg0, nullptr};
            app = new QApplication(argc, argv);
        }
        commandLine = new CommandLine();
    }
    
    void TearDown() override {
        delete commandLine;
    }
    
    CommandLine* commandLine;
    static QApplication* app;
};

QApplication* CommandLineTest::app = nullptr;

TEST_F(CommandLineTest, InitialState) {
    EXPECT_NE(commandLine, nullptr);
}

TEST_F(CommandLineTest, AddOutput) {
    QString testOutput = "Test output message";
    commandLine->addOutput(testOutput);
    
    // Should not crash
    SUCCEED();
}

TEST_F(CommandLineTest, ClearOutput) {
    commandLine->addOutput("Test message");
    commandLine->clearOutput();
    
    // Should not crash
    SUCCEED();
}

TEST_F(CommandLineTest, CommandExecution) {
    QSignalSpy spy(commandLine, &CommandLine::commandExecuted);
    
    // Simulate entering a command - this would normally be done through UI
    // For testing, we'll just verify the signal exists
    EXPECT_EQ(spy.count(), 0);
}

class MainWindowTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 1;
            static char arg0[] = "stdgeo_tests";
            static char* argv[] = {arg0, nullptr};
            app = new QApplication(argc, argv);
        }
        mainWindow = new MainWindow();
    }
    
    void TearDown() override {
        delete mainWindow;
    }
    
    MainWindow* mainWindow;
    static QApplication* app;
};

QApplication* MainWindowTest::app = nullptr;

TEST_F(MainWindowTest, InitialState) {
    EXPECT_NE(mainWindow, nullptr);
    EXPECT_NE(mainWindow->windowTitle().isEmpty(), true);
}

TEST_F(MainWindowTest, MenuActions) {
    // Test that menu actions exist - don't show window in tests
    // We can't easily test private methods, but we can verify the window was created
    EXPECT_NE(mainWindow->menuBar(), nullptr);
    EXPECT_NE(mainWindow->statusBar(), nullptr);
}

// Integration tests
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 1;
            static char arg0[] = "stdgeo_tests";
            static char* argv[] = {arg0, nullptr};
            app = new QApplication(argc, argv);
        }
        mainWindow = new MainWindow();
    }
    
    void TearDown() override {
        delete mainWindow;
    }
    
    MainWindow* mainWindow;
    static QApplication* app;
};

QApplication* IntegrationTest::app = nullptr;

TEST_F(IntegrationTest, ApplicationStartup) {
    // Test that the application can start up without crashing - don't show window
    
    // Verify main components exist
    EXPECT_NE(mainWindow->findChild<GeometryViewer*>(), nullptr);
    EXPECT_NE(mainWindow->findChild<CommandLine*>(), nullptr);
}

TEST_F(IntegrationTest, ViewerInteraction) {
    // Get the viewer component - don't show window
    GeometryViewer* viewer = mainWindow->findChild<GeometryViewer*>();
    ASSERT_NE(viewer, nullptr);
    
    // Test that we can create a cube through the main window
    // This would normally be done through menu or command line
    viewer->createCube(1.0);
    
    SUCCEED();
}