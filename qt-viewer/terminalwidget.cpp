#include "terminalwidget.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>
#include <QSplitter>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_outputDisplay(nullptr)
    , m_commandLine(nullptr)
    , m_sessionButton(nullptr)
    , m_clearButton(nullptr)
    , m_statusLabel(nullptr)
#ifdef HAVE_QTERMWIDGET
    , m_nativeTerminal(nullptr)
#endif
    , m_useNativeTerminal(false)
    , m_cliProcess(nullptr)
    , m_sessionMode(false)
    , m_outputTimer(new QTimer(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_historyIndex(-1)
    , m_outputColor(Qt::black)
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
}

TerminalWidget::~TerminalWidget()
{
    if (m_cliProcess && m_cliProcess->state() != QProcess::NotRunning) {
        m_cliProcess->kill();
        m_cliProcess->waitForFinished(1000);
    }
}

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
    
#ifdef HAVE_QTERMWIDGET
    // Try to use native terminal if available
    m_nativeTerminal = new QTermWidget(this);
    if (m_nativeTerminal) {
        m_useNativeTerminal = true;
        m_nativeTerminal->setShellProgram(m_stdgeoBinaryPath);
        m_nativeTerminal->setArgs(QStringList() << "session");
        m_nativeTerminal->setColorScheme("Linux");
        m_nativeTerminal->setScrollBarPosition(QTermWidget::ScrollBarRight);
        m_mainLayout->addWidget(m_nativeTerminal);
    } else {
        m_useNativeTerminal = false;
    }
#endif

    if (!m_useNativeTerminal) {
        // Fallback to custom terminal emulation
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
    }
    
    // Connect signals
    connect(m_sessionButton, &QPushButton::clicked, this, &TerminalWidget::onSessionButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &TerminalWidget::onClearButtonClicked);
    
    if (m_commandLine) {
        connect(m_commandLine, &QLineEdit::returnPressed, this, &TerminalWidget::onCommandLineReturnPressed);
    }
}

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

void TerminalWidget::startInteractiveSession()
{
    if (m_cliProcess->state() != QProcess::NotRunning) {
        return;
    }
    
#ifdef HAVE_QTERMWIDGET
    if (m_useNativeTerminal && m_nativeTerminal) {
        m_nativeTerminal->startShellProgram();
        m_sessionMode = true;
        updateSessionButton();
        emit sessionStarted();
        return;
    }
#endif

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

void TerminalWidget::stopSession()
{
#ifdef HAVE_QTERMWIDGET
    if (m_useNativeTerminal && m_nativeTerminal) {
        // Native terminal handles session management internally
        m_sessionMode = false;
        updateSessionButton();
        emit sessionEnded();
        return;
    }
#endif

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

void TerminalWidget::executeCommand(const QString &command)
{
    if (command.isEmpty()) return;
    
#ifdef HAVE_QTERMWIDGET
    if (m_useNativeTerminal && m_nativeTerminal) {
        m_nativeTerminal->sendText(command + "\n");
        return;
    }
#endif

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

void TerminalWidget::clear()
{
    if (m_outputDisplay) {
        m_outputDisplay->clear();
        appendPrompt();
    }
}

bool TerminalWidget::isCliRunning() const
{
#ifdef HAVE_QTERMWIDGET
    if (m_useNativeTerminal) {
        return m_sessionMode;
    }
#endif
    return m_cliProcess && m_cliProcess->state() == QProcess::Running;
}

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

void TerminalWidget::onReadyReadStandardOutput()
{
    QByteArray data = m_cliProcess->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(data);
    appendOutput(output, m_outputColor);
}

void TerminalWidget::onReadyReadStandardError()
{
    QByteArray data = m_cliProcess->readAllStandardError();
    QString error = QString::fromLocal8Bit(data);
    appendOutput(error, m_errorColor);
}

void TerminalWidget::onCommandLineReturnPressed()
{
    if (!m_commandLine) return;
    
    QString command = m_commandLine->text().trimmed();
    m_commandLine->clear();
    
    if (!command.isEmpty()) {
        executeCommand(command);
    }
}

void TerminalWidget::onClearButtonClicked()
{
    clear();
}

void TerminalWidget::onSessionButtonClicked()
{
    if (m_sessionMode) {
        stopSession();
    } else {
        startInteractiveSession();
    }
}

void TerminalWidget::watchForGeometryFiles()
{
    // This is called periodically to check for geometry file changes
    // In a real implementation, this could trigger reloads in the main window
}

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

void TerminalWidget::appendPrompt()
{
    if (m_outputDisplay && !m_sessionMode) {
        appendOutput("stdgeo> ", m_promptColor);
    }
}

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

void TerminalWidget::keyPressEvent(QKeyEvent *event)
{
    if (!m_commandLine) {
        QWidget::keyPressEvent(event);
        return;
    }
    
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