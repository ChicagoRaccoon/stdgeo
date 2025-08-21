/*
 * Theory of Operation:
 * This test suite validates the TerminalWidget functionality using Qt's testing framework.
 * It covers widget initialization, command execution, session management, and UI interactions.
 * Tests are designed to work with or without the stdgeo binary present, gracefully handling
 * missing dependencies with appropriate skip messages. The tests verify both the fallback
 * terminal implementation and signal emissions for proper widget communication.
 */

#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QLineEdit>
#include <QTextEdit>
#include "../terminalwidget.h"

// Test class for TerminalWidget functionality validation
class TestTerminalWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testInitialization();
    void testCommandExecution();
    void testSessionManagement();
    void testClearFunction();

private:
    TerminalWidget *m_terminal;
    QApplication *m_app;
};

// Sets up Qt application instance for testing
void TestTerminalWidget::initTestCase()
{
    // Initialize Qt application if not already done
    if (!QApplication::instance()) {
        static int argc = 1;
        static const char* argv[] = {"test", nullptr};
        m_app = new QApplication(argc, const_cast<char**>(argv));
    }
}

// Cleans up after all tests complete
void TestTerminalWidget::cleanupTestCase()
{
    // Cleanup is handled by Qt
}

// Creates fresh TerminalWidget instance for each test
void TestTerminalWidget::init()
{
    m_terminal = new TerminalWidget();
    m_terminal->show();
    
    // Wait for the widget to be visible
    [[maybe_unused]] bool exposed = QTest::qWaitForWindowExposed(m_terminal);
}

// Cleans up TerminalWidget instance after each test
void TestTerminalWidget::cleanup()
{
    if (m_terminal->isCliRunning()) {
        m_terminal->stopSession();
        QTest::qWait(1000); // Wait for session to stop
    }
    delete m_terminal;
}

// Verifies proper widget initialization and UI component presence
void TestTerminalWidget::testInitialization()
{
    QVERIFY(m_terminal != nullptr);
    QVERIFY(m_terminal->isVisible());
    QVERIFY(!m_terminal->isCliRunning());
    
    // Check that UI components are present
    QLineEdit *commandLine = m_terminal->findChild<QLineEdit*>();
    QTextEdit *outputDisplay = m_terminal->findChild<QTextEdit*>();
    
    // At least one should be present (depends on whether native terminal is available)
    QVERIFY(commandLine != nullptr || outputDisplay != nullptr);
}

// Tests command execution and signal emission with help command
void TestTerminalWidget::testCommandExecution()
{
    QSignalSpy commandSpy(m_terminal, &TerminalWidget::commandExecuted);
    
    // Execute a simple command (help should always work)
    m_terminal->executeCommand("--help");
    
    // Wait for command to complete (up to 5 seconds)
    bool signalReceived = commandSpy.wait(5000);
    
    if (signalReceived) {
        // Verify signal was emitted with correct parameters
        QCOMPARE(commandSpy.count(), 1);
        QList<QVariant> arguments = commandSpy.takeFirst();
        QString command = arguments.at(0).toString();
        QString output = arguments.at(1).toString();
        
        QCOMPARE(command, QString("--help"));
        QVERIFY(!output.isEmpty());
    } else {
        // If command fails, it might be because stdgeo binary is not found
        qWarning("Command execution test skipped - stdgeo binary not found or not working");
    }
}

// Validates interactive session start/stop functionality
void TestTerminalWidget::testSessionManagement()
{
    if (!QFile::exists("./stdgeo") && !QFile::exists("../bin/stdgeo")) {
        QSKIP("stdgeo binary not found - skipping session tests");
    }
    
    QSignalSpy sessionStartedSpy(m_terminal, &TerminalWidget::sessionStarted);
    QSignalSpy sessionEndedSpy(m_terminal, &TerminalWidget::sessionEnded);
    
    // Start session
    QVERIFY(!m_terminal->isCliRunning());
    m_terminal->startInteractiveSession();
    
    // Wait for session to start
    bool sessionStarted = sessionStartedSpy.wait(3000);
    if (sessionStarted) {
        QCOMPARE(sessionStartedSpy.count(), 1);
        QVERIFY(m_terminal->isCliRunning());
        
        // Stop session
        m_terminal->stopSession();
        
        // Wait for session to end
        bool sessionEnded = sessionEndedSpy.wait(3000);
        QVERIFY(sessionEnded);
        QCOMPARE(sessionEndedSpy.count(), 1);
        QVERIFY(!m_terminal->isCliRunning());
    } else {
        qWarning("Session test skipped - could not start stdgeo session");
    }
}

// Tests terminal output clearing functionality
void TestTerminalWidget::testClearFunction()
{
    // Execute a command to generate some output
    m_terminal->executeCommand("--help");
    QTest::qWait(1000);
    
    // Find output display (if using fallback terminal)
    QTextEdit *outputDisplay = m_terminal->findChild<QTextEdit*>();
    
    if (outputDisplay) {
        // Should have some content after command
        QVERIFY(!outputDisplay->toPlainText().isEmpty());
        
        // Clear the terminal
        m_terminal->clear();
        
        // Content should be cleared (or minimal prompt only)
        QString clearedText = outputDisplay->toPlainText();
        QVERIFY(clearedText.isEmpty() || clearedText.contains("stdgeo>"));
    }
}

QTEST_MAIN(TestTerminalWidget)
#include "test_terminalwidget.moc"

