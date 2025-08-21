/*
 * Theory of Operation:
 * TerminalWidget provides an integrated CLI interface for the StdGeo application.
 * It supports two modes of operation:
 * 1. Native terminal (if QTermWidget is available) - provides full terminal emulation
 * 2. Fallback custom terminal - uses QTextEdit for output and QLineEdit for input
 * 
 * The widget manages process execution for the stdgeo CLI binary, handles both
 * interactive sessions and single command execution, maintains command history,
 * and provides file system watching for geometry file changes. It automatically
 * locates the stdgeo binary in various standard locations and provides visual
 * feedback through status indicators.
 */

#include "terminalwidget.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>
#include <QSplitter>

// Constructor - sets up UI layout, terminal process, and file watching
TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_outputDisplay(nullptr)
    , m_commandLine(nullptr)
    , m_sessionButton(nullptr)
    , m_clearButton(nullptr)
    , m_statusLabel(nullptr)
    , m_cliProcess(nullptr)
    , m_sessionMode(false)
    , m_outputTimer(new QTimer(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_historyIndex(-1)
    , m_outputColor(Qt::white)
    , m_errorColor(Qt::red)
    , m_promptColor(Qt::blue)
    , m_commandColor(Qt::darkGreen)
{
    setupUI();
    setupTerminal();
    setupFileWatcher();
    
    // Find the stdgeo binary
    m_stdgeoBinaryPath = findStdgeoBinary();
    
    // Setup output polling timer
    m_outputTimer->setInterval(50);
    connect(m_outputTimer, &QTimer::timeout, this, &TerminalWidget::watchForGeometryFiles);
    
    // Auto-start interactive session
    QTimer::singleShot(100, this, &TerminalWidget::startInteractiveSession);
}

// Constructor with shared session - uses existing QProcess instead of creating new one
TerminalWidget::TerminalWidget(QProcess *sharedSession, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_outputDisplay(nullptr)
    , m_commandLine(nullptr)
    , m_sessionButton(nullptr)
    , m_clearButton(nullptr)
    , m_statusLabel(nullptr)
    // Using QProcess-based implementation only
    , m_cliProcess(sharedSession)
    , m_sessionMode(true)  // Session is already active
    , m_outputTimer(new QTimer(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_historyIndex(-1)
    , m_outputColor(Qt::white)
    , m_errorColor(Qt::red)
    , m_promptColor(Qt::blue)
    , m_commandColor(Qt::darkGreen)
{
    setupUI();
    setupSharedTerminal();  // Different setup for shared session
    setupFileWatcher();
    
    // Find the stdgeo binary (for display purposes)
    m_stdgeoBinaryPath = findStdgeoBinary();
    
    // Setup output polling timer
    m_outputTimer->setInterval(50);
    connect(m_outputTimer, &QTimer::timeout, this, &TerminalWidget::watchForGeometryFiles);
    
    // Session is already running, so update UI accordingly
    updateSessionButton();
    m_outputTimer->start();
}

// Destructor - terminates CLI process and cleans up resources
TerminalWidget::~TerminalWidget()
{
    if (m_cliProcess && m_cliProcess->state() != QProcess::NotRunning) {
        m_cliProcess->kill();
        m_cliProcess->waitForFinished(1000);
    }
}

// Creates the user interface layout with control buttons and terminal display
void TerminalWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Control buttons layout
    m_controlLayout = new QHBoxLayout();
    
    m_sessionButton = new QPushButton("Start Session", this);
    m_clearButton = new QPushButton("Clear", this);
    m_statusLabel = new QLabel("Ready", this);
    
    m_controlLayout->addWidget(m_sessionButton);
    m_controlLayout->addWidget(m_clearButton);
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addLayout(m_controlLayout);
    
    // QProcess-based terminal implementation
    m_outputDisplay = new QTextEdit(this);
    m_outputDisplay->setReadOnly(true);
    m_outputDisplay->setFont(QFont("Consolas", 10));
    m_outputDisplay->setStyleSheet(
        "QTextEdit {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: 1px solid #555555;"
        "}"
    );
    
    m_commandLine = new QLineEdit(this);
    m_commandLine->setFont(QFont("Consolas", 10));
    m_commandLine->setStyleSheet(
        "QLineEdit {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "    border: 1px solid #555555;"
        "    padding: 5px;"
        "}"
    );
    m_commandLine->setPlaceholderText("Enter stdgeo command...");
    
    m_mainLayout->addWidget(m_outputDisplay, 1);
    m_mainLayout->addWidget(m_commandLine);
    
    // Connect signals
    connect(m_sessionButton, &QPushButton::clicked, this, &TerminalWidget::onSessionButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &TerminalWidget::onClearButtonClicked);
    connect(m_commandLine, &QLineEdit::returnPressed, this, &TerminalWidget::onCommandLineReturnPressed);
}

// Initializes QProcess for CLI communication and connects process signals
void TerminalWidget::setupTerminal()
{
    m_cliProcess = new QProcess(this);
    
    connect(m_cliProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
    connect(m_cliProcess, &QProcess::errorOccurred, this, &TerminalWidget::onProcessError);
    connect(m_cliProcess, &QProcess::readyReadStandardOutput, this, &TerminalWidget::onReadyReadStandardOutput);
    connect(m_cliProcess, &QProcess::readyReadStandardError, this, &TerminalWidget::onReadyReadStandardError);
    
    m_terminalFont = QFont("Consolas", 10);
    if (!m_terminalFont.exactMatch()) {
        m_terminalFont = QFont("Courier New", 10);
    }
}

// Connects to existing shared QProcess for CLI communication
void TerminalWidget::setupSharedTerminal()
{
    // m_cliProcess is already set to the shared session in constructor
    // Just connect to its signals
    connect(m_cliProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
    connect(m_cliProcess, &QProcess::errorOccurred, this, &TerminalWidget::onProcessError);
    connect(m_cliProcess, &QProcess::readyReadStandardOutput, this, &TerminalWidget::onReadyReadStandardOutput);
    connect(m_cliProcess, &QProcess::readyReadStandardError, this, &TerminalWidget::onReadyReadStandardError);
    
    m_terminalFont = QFont("Consolas", 10);
    if (!m_terminalFont.exactMatch()) {
        m_terminalFont = QFont("Courier New", 10);
    }
}

// Configures file system monitoring for geometry file changes
void TerminalWidget::setupFileWatcher()
{
    // Watch current directory for geometry files
    m_fileWatcher->addPath(QDir::currentPath());
    
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, 
            [this](const QString &path) {
                Q_UNUSED(path)
                // File changed - could trigger a refresh in the main window
                qDebug() << "Geometry file changed:" << path;
            });
    
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            [this](const QString &path) {
                Q_UNUSED(path)
                // Directory changed - scan for new .json files
                QDir dir(path);
                QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
                for (const QString &file : jsonFiles) {
                    QString fullPath = dir.absoluteFilePath(file);
                    if (!m_watchedFiles.contains(fullPath)) {
                        m_fileWatcher->addPath(fullPath);
                        m_watchedFiles.append(fullPath);
                    }
                }
            });
}

// Searches for stdgeo executable in build directories and system PATH
QString TerminalWidget::findStdgeoBinary()
{
    // Look for the stdgeo binary in various locations
    QStringList searchPaths;
    
    // First try the build directory
    QString buildDir = QDir::currentPath() + "/build/bin/stdgeo";
    if (QFile::exists(buildDir)) {
        return buildDir;
    }
    
    // Try relative to current application
    QString appDir = QApplication::applicationDirPath();
    QStringList relativePaths = {
        appDir + "/stdgeo-cli",
        appDir + "/stdgeo",
        appDir + "/../bin/stdgeo",
        appDir + "/../../bin/stdgeo"
    };
    
    for (const QString &path : relativePaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    // Try system PATH
    return "stdgeo"; // Will use system PATH
}

// Launches stdgeo in interactive session mode for continuous CLI interaction
void TerminalWidget::startInteractiveSession()
{
    if (m_cliProcess->state() != QProcess::NotRunning) {
        return;
    }
    
    appendOutput("Starting stdgeo interactive session...\n", m_promptColor);
    
    m_cliProcess->start(m_stdgeoBinaryPath, QStringList() << "session");
    
    if (!m_cliProcess->waitForStarted(3000)) {
        appendOutput("Error: Could not start stdgeo CLI\n", m_errorColor);
        appendOutput("Make sure stdgeo is built and available\n", m_errorColor);
        return;
    }
    
    m_sessionMode = true;
    updateSessionButton();
    m_outputTimer->start();
    emit sessionStarted();
}

// Terminates the current interactive session gracefully
void TerminalWidget::stopSession()
{
    if (m_cliProcess->state() != QProcess::NotRunning) {
        sendCommand("quit");
        if (!m_cliProcess->waitForFinished(2000)) {
            m_cliProcess->kill();
            m_cliProcess->waitForFinished(1000);
        }
    }
    
    m_sessionMode = false;
    m_outputTimer->stop();
    updateSessionButton();
    appendOutput("\nSession ended.\n", m_promptColor);
    emit sessionEnded();
}

// Executes a single command either in session mode or as standalone process
void TerminalWidget::executeCommand(const QString &command)
{
    if (command.isEmpty()) return;
    
    if (m_sessionMode && m_cliProcess->state() == QProcess::Running) {
        sendCommand(command);
    } else {
        // Execute single command
        appendOutput(QString("$ stdgeo %1\n").arg(command), m_commandColor);
        
        QProcess singleCommand;
        singleCommand.start(m_stdgeoBinaryPath, command.split(' ', Qt::SkipEmptyParts));
        
        if (singleCommand.waitForFinished(5000)) {
            QString output = singleCommand.readAllStandardOutput();
            QString error = singleCommand.readAllStandardError();
            
            if (!output.isEmpty()) {
                appendOutput(output, m_outputColor);
            }
            if (!error.isEmpty()) {
                appendOutput(error, m_errorColor);
            }
            
            emit commandExecuted(command, output + error);
        } else {
            appendOutput("Command timed out or failed to execute\n", m_errorColor);
        }
        
        appendPrompt();
    }
}

// Sends command to running CLI process and updates command history
void TerminalWidget::sendCommand(const QString &command)
{
    if (m_cliProcess->state() == QProcess::Running) {
        m_cliProcess->write(command.toLocal8Bit() + "\n");
        
        // Add to command history
        if (!command.isEmpty() && (m_commandHistory.isEmpty() || m_commandHistory.last() != command)) {
            m_commandHistory.append(command);
            if (m_commandHistory.size() > 100) { // Limit history size
                m_commandHistory.removeFirst();
            }
        }
        m_historyIndex = m_commandHistory.size();
    }
}

// Clears the terminal output display
void TerminalWidget::clear()
{
    if (!m_outputDisplay) { return; }
    m_outputDisplay->clear();
    appendPrompt();
}

// Returns true if CLI process is currently running
bool TerminalWidget::isCliRunning() const
{
    return m_cliProcess && m_cliProcess->state() == QProcess::Running;
}

// Handles CLI process termination and updates session state
void TerminalWidget::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    
    if (m_sessionMode) {
        appendOutput("\nSession ended.\n", m_promptColor);
        m_sessionMode = false;
        updateSessionButton();
        m_outputTimer->stop();
        emit sessionEnded();
    }
}

// Handles CLI process errors and displays appropriate error messages
void TerminalWidget::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start stdgeo CLI. Check if the binary exists and is executable.";
            break;
        case QProcess::Crashed:
            errorMsg = "stdgeo CLI crashed.";
            break;
        case QProcess::Timedout:
            errorMsg = "stdgeo CLI timed out.";
            break;
        default:
            errorMsg = "Unknown error occurred with stdgeo CLI.";
    }
    
    appendOutput(errorMsg + "\n", m_errorColor);
    
    if (m_sessionMode) {
        m_sessionMode = false;
        updateSessionButton();
        m_outputTimer->stop();
        emit sessionEnded();
    }
}

// Reads and displays standard output from CLI process
void TerminalWidget::onReadyReadStandardOutput()
{
    QByteArray data = m_cliProcess->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data);
    appendOutput(output, m_outputColor);
}

// Reads and displays error output from CLI process
void TerminalWidget::onReadyReadStandardError()
{
    QByteArray data = m_cliProcess->readAllStandardError();
    QString error = QString::fromLocal8Bit(data);
    appendOutput(error, m_errorColor);
}

// Handles Enter key press in command line input field
void TerminalWidget::onCommandLineReturnPressed()
{
    if (!m_commandLine) return;
    
    QString command = m_commandLine->text().trimmed();
    m_commandLine->clear();
    
    if (!command.isEmpty()) {
        executeCommand(command);
    }
}

// Handles clear button click event
void TerminalWidget::onClearButtonClicked()
{
    clear();
}

// Toggles interactive session state when session button is clicked
void TerminalWidget::onSessionButtonClicked()
{
    if (m_sessionMode) {
        stopSession();
    } else {
        startInteractiveSession();
    }
}

// Periodic callback to monitor geometry file changes
void TerminalWidget::watchForGeometryFiles()
{
    // This is called periodically to check for geometry file changes
    // In a real implementation, this could trigger reloads in the main window
}

// Appends colored text to the terminal output display
void TerminalWidget::appendOutput(const QString &text, const QColor &color)
{
    if (!m_outputDisplay) return;
    
    QTextCursor cursor(m_outputDisplay->document());
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(color);
    cursor.setCharFormat(format);
    cursor.insertText(text);
    
    // Auto-scroll to bottom
    QScrollBar *scrollBar = m_outputDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

// Displays command prompt in terminal output
void TerminalWidget::appendPrompt()
{
    if (!m_sessionMode) {
        appendOutput("> ", m_promptColor);
    }
}

// Updates session button text and status label based on current state
void TerminalWidget::updateSessionButton()
{
    if (m_sessionMode) {
        m_sessionButton->setText("Stop Session");
        m_statusLabel->setText("Session Active");
    } else {
        m_sessionButton->setText("Start Session");
        m_statusLabel->setText("Ready");
    }
}

// Handles keyboard events for command history navigation
void TerminalWidget::keyPressEvent(QKeyEvent *event)
{
    // Handle command history navigation
    if (event->key() == Qt::Key_Up) {
        if (m_historyIndex > 0) {
            m_historyIndex--;
            m_commandLine->setText(m_commandHistory.at(m_historyIndex));
        }
        return;
    } else if (event->key() == Qt::Key_Down) {
        if (m_historyIndex < m_commandHistory.size() - 1) {
            m_historyIndex++;
            m_commandLine->setText(m_commandHistory.at(m_historyIndex));
        } else if (m_historyIndex == m_commandHistory.size() - 1) {
            m_historyIndex = m_commandHistory.size();
            m_commandLine->clear();
        }
        return;
    }
    
    QWidget::keyPressEvent(event);
}