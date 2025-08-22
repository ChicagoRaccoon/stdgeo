/*
 * Theory of Operation:
 * TerminalWidget provides an integrated CLI interface for the StdGeo application.
 * It now uses the stdgeo-parser library directly instead of spawning external processes,
 * providing better performance and tighter integration between the GUI and CLI functionality.
 * 
 * The widget maintains a shared geometry context that can be accessed by both the
 * terminal interface and the visual interface, enabling real-time synchronization
 * between command execution and visual display.
 */

#include "terminalwidget.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextCursor>
#include <QScrollBar>
#include <QDebug>
#include <QSplitter>

// Constructor - sets up UI layout, parser, and file watching
TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_outputDisplay(nullptr)
    , m_commandLine(nullptr)
    , m_sessionButton(nullptr)
    , m_clearButton(nullptr)
    , m_statusLabel(nullptr)
    , m_parser(nullptr)
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
    setupParser();
    setupFileWatcher();
    
    // Setup output polling timer for file watching
    m_outputTimer->setInterval(50);
    connect(m_outputTimer, &QTimer::timeout, this, &TerminalWidget::watchForGeometryFiles);
    
    // Auto-start interactive session
    QTimer::singleShot(100, this, &TerminalWidget::startInteractiveSession);
}

// Constructor with shared parser - uses existing parser instance
TerminalWidget::TerminalWidget(StdGeoParser *sharedParser, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_outputDisplay(nullptr)
    , m_commandLine(nullptr)
    , m_sessionButton(nullptr)
    , m_clearButton(nullptr)
    , m_statusLabel(nullptr)
    , m_parser(sharedParser)
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
    setupSharedParser();
    setupFileWatcher();
    
    // Setup output polling timer
    m_outputTimer->setInterval(50);
    connect(m_outputTimer, &QTimer::timeout, this, &TerminalWidget::watchForGeometryFiles);
    
    // Session is already running, so update UI accordingly
    updateSessionButton();
    m_outputTimer->start();
}

// Destructor - cleans up parser if we own it
TerminalWidget::~TerminalWidget()
{
    // Only delete parser if we own it (not shared)
    if (m_parser && !m_sessionMode) {
        delete m_parser;
    }
}

// Sets up the user interface components and layout
void TerminalWidget::setupUI()
{
    setStyleSheet("QWidget { background-color: #1e1e1e; color: #ffffff; }");
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Output display
    m_outputDisplay = new QTextEdit(this);
    m_outputDisplay->setReadOnly(true);
    m_outputDisplay->setFont(QFont("Consolas", 10));
    m_outputDisplay->setStyleSheet(
        "QTextEdit { "
        "background-color: #2d2d2d; "
        "color: #ffffff; "
        "border: 1px solid #555; "
        "}"
    );
    
    // Command input line
    m_commandLine = new QLineEdit(this);
    m_commandLine->setFont(QFont("Consolas", 10));
    m_commandLine->setStyleSheet(
        "QLineEdit { "
        "background-color: #2d2d2d; "
        "color: #ffffff; "
        "border: 1px solid #555; "
        "padding: 5px; "
        "}"
    );
    
    // Control buttons layout
    m_controlLayout = new QHBoxLayout();
    
    m_sessionButton = new QPushButton("Start Session", this);
    m_clearButton = new QPushButton("Clear", this);
    
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { color: #00ff00; }");
    
    m_controlLayout->addWidget(m_sessionButton);
    m_controlLayout->addWidget(m_clearButton);
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_statusLabel);
    
    // Add to main layout
    m_mainLayout->addWidget(m_outputDisplay);
    m_mainLayout->addWidget(m_commandLine);
    m_mainLayout->addLayout(m_controlLayout);
    
    // Connect signals
    connect(m_commandLine, &QLineEdit::returnPressed, this, &TerminalWidget::onCommandLineReturnPressed);
    connect(m_sessionButton, &QPushButton::clicked, this, &TerminalWidget::onSessionButtonClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &TerminalWidget::onClearButtonClicked);
    
    appendOutput("StdGeo Terminal Widget - Ready\n", m_promptColor);
    appendPrompt();
}

// Sets up the parser for this widget
void TerminalWidget::setupParser()
{
    m_parser = new StdGeoParser(this);
    
    if (!m_parser->isValid()) {
        appendOutput("Error: Failed to initialize stdgeo parser\n", m_errorColor);
        m_statusLabel->setText("Parser Error");
        m_statusLabel->setStyleSheet("QLabel { color: #ff0000; }");
        return;
    }
    
    // Connect parser signals
    connect(m_parser, &StdGeoParser::commandExecuted, this, &TerminalWidget::onCommandExecuted);
    
    m_statusLabel->setText("Parser Ready");
    m_statusLabel->setStyleSheet("QLabel { color: #00ff00; }");
}

// Sets up connections for shared parser
void TerminalWidget::setupSharedParser()
{
    if (!m_parser || !m_parser->isValid()) {
        appendOutput("Error: Invalid shared parser\n", m_errorColor);
        m_statusLabel->setText("Parser Error");
        m_statusLabel->setStyleSheet("QLabel { color: #ff0000; }");
        return;
    }
    
    // Connect parser signals
    connect(m_parser, &StdGeoParser::commandExecuted, this, &TerminalWidget::onCommandExecuted);
    
    m_statusLabel->setText("Shared Parser");
    m_statusLabel->setStyleSheet("QLabel { color: #ffff00; }");
}

// Sets up file system watching for geometry files
void TerminalWidget::setupFileWatcher()
{
    // Watch current directory for geometry files
    QStringList filters;
    filters << "*.json" << "*.txt";
    
    QString currentDir = QDir::currentPath();
    if (QDir(currentDir).exists()) {
        m_fileWatcher->addPath(currentDir);
    }
    
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &TerminalWidget::watchForGeometryFiles);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &TerminalWidget::watchForGeometryFiles);
}

// Starts an interactive session
void TerminalWidget::startInteractiveSession()
{
    if (!m_parser || !m_parser->isValid()) {
        appendOutput("Cannot start session: Parser not ready\n", m_errorColor);
        return;
    }
    
    m_sessionMode = true;
    updateSessionButton();
    m_outputTimer->start();
    
    appendOutput("Interactive session started. Type 'help' for commands, 'quit' to exit.\n", m_promptColor);
    appendPrompt();
    
    emit sessionStarted();
}

// Stops the interactive session
void TerminalWidget::stopSession()
{
    if (m_sessionMode) {
        m_sessionMode = false;
        updateSessionButton();
        m_outputTimer->stop();
        
        appendOutput("Session ended.\n", m_promptColor);
        emit sessionEnded();
    }
}

// Executes a single command
void TerminalWidget::executeCommand(const QString &command)
{
    if (command.isEmpty()) return;
    
    if (!m_parser || !m_parser->isValid()) {
        appendOutput("Error: Parser not ready\n", m_errorColor);
        return;
    }
    
    appendOutput(QString("$ %1\n").arg(command), m_commandColor);
    
    // Execute command via parser
    QString result = m_parser->executeCommand(command);
    
    // Result will be handled by onCommandExecuted slot
}

// Sends command (alias for executeCommand in parser mode)
void TerminalWidget::sendCommand(const QString &command)
{
    executeCommand(command);
}

// Clears the terminal output display
void TerminalWidget::clear()
{
    if (!m_outputDisplay) { return; }
    m_outputDisplay->clear();
    appendOutput("StdGeo Terminal Widget - Ready\n", m_promptColor);
    appendPrompt();
}

// Returns true if parser is ready
bool TerminalWidget::isParserReady() const
{
    return m_parser && m_parser->isValid();
}

// Handles command execution results from parser
void TerminalWidget::onCommandExecuted(const QString &command, const QString &result, bool success)
{
    if (!result.isEmpty()) {
        QColor outputColor = success ? m_outputColor : m_errorColor;
        appendOutput(result + "\n", outputColor);
    }
    
    emit commandExecuted(command, result);
    appendPrompt();
}

// Handles return key press in command line
void TerminalWidget::onCommandLineReturnPressed()
{
    QString command = m_commandLine->text().trimmed();
    m_commandLine->clear();
    
    if (command.isEmpty()) {
        appendPrompt();
        return;
    }
    
    // Add to command history
    if (!command.isEmpty() && (m_commandHistory.isEmpty() || m_commandHistory.last() != command)) {
        m_commandHistory.append(command);
        if (m_commandHistory.size() > 100) { // Limit history size
            m_commandHistory.removeFirst();
        }
    }
    m_historyIndex = m_commandHistory.size();
    
    // Handle special session commands
    if (command == "quit" || command == "exit") {
        if (m_sessionMode) {
            stopSession();
        }
        return;
    }
    
    executeCommand(command);
}

// Handles clear button click
void TerminalWidget::onClearButtonClicked()
{
    clear();
}

// Handles session button click
void TerminalWidget::onSessionButtonClicked()
{
    if (m_sessionMode) {
        stopSession();
    } else {
        startInteractiveSession();
    }
}

// Updates session button text and state
void TerminalWidget::updateSessionButton()
{
    if (m_sessionButton) {
        if (m_sessionMode) {
            m_sessionButton->setText("Stop Session");
        } else {
            m_sessionButton->setText("Start Session");
        }
    }
}

// Appends text to output display with specified color
void TerminalWidget::appendOutput(const QString &text, const QColor &color)
{
    if (!m_outputDisplay) return;
    
    QTextCursor cursor = m_outputDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(color);
    cursor.setCharFormat(format);
    cursor.insertText(text);
    
    m_outputDisplay->setTextCursor(cursor);
    m_outputDisplay->ensureCursorVisible();
    
    // Auto-scroll to bottom
    QScrollBar *scrollBar = m_outputDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

// Appends command prompt
void TerminalWidget::appendPrompt()
{
    if (m_sessionMode) {
        appendOutput("stdgeo> ", m_promptColor);
    }
}

// Watches for geometry file changes
void TerminalWidget::watchForGeometryFiles()
{
    // This method can be used to detect file changes and update the GUI
    // For now, it's a placeholder for future file watching functionality
}

// Handles key press events for command history navigation
void TerminalWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up && !m_commandHistory.isEmpty()) {
        if (m_historyIndex > 0) {
            m_historyIndex--;
            m_commandLine->setText(m_commandHistory[m_historyIndex]);
        }
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_Down && !m_commandHistory.isEmpty()) {
        if (m_historyIndex < m_commandHistory.size() - 1) {
            m_historyIndex++;
            m_commandLine->setText(m_commandHistory[m_historyIndex]);
        } else {
            m_historyIndex = m_commandHistory.size();
            m_commandLine->clear();
        }
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);
}