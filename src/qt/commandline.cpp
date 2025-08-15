#include "commandline.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QFont>

CommandLine::CommandLine(QWidget *parent)
    : QWidget(parent)
    , m_outputArea(nullptr)
    , m_inputLine(nullptr)
    , m_executeButton(nullptr)
    , m_clearButton(nullptr)
    , m_historyIndex(-1)
{
    setupUI();
}

void CommandLine::setupUI()
{
    setMinimumHeight(150);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Output area
    QLabel *outputLabel = new QLabel("Output:", this);
    mainLayout->addWidget(outputLabel);
    
    m_outputArea = new QTextEdit(this);
    m_outputArea->setReadOnly(true);
    m_outputArea->setFont(QFont("Consolas", 10));
    m_outputArea->setStyleSheet(
        "QTextEdit {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "}"
    );
    mainLayout->addWidget(m_outputArea);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    QLabel *inputLabel = new QLabel("Command:", this);
    inputLayout->addWidget(inputLabel);
    
    m_inputLine = new QLineEdit(this);
    m_inputLine->setFont(QFont("Consolas", 10));
    m_inputLine->setStyleSheet(
        "QLineEdit {"
        "    background-color: #1e1e1e;"
        "    color: #d4d4d4;"
        "    border: 1px solid #555555;"
        "    border-radius: 3px;"
        "    padding: 3px;"
        "}"
    );
    connect(m_inputLine, &QLineEdit::returnPressed, this, &CommandLine::onReturnPressed);
    inputLayout->addWidget(m_inputLine);
    
    m_executeButton = new QPushButton("Execute", this);
    connect(m_executeButton, &QPushButton::clicked, this, &CommandLine::executeCommand);
    inputLayout->addWidget(m_executeButton);
    
    m_clearButton = new QPushButton("Clear", this);
    connect(m_clearButton, &QPushButton::clicked, this, &CommandLine::clearOutput);
    inputLayout->addWidget(m_clearButton);
    
    mainLayout->addLayout(inputLayout);
    
    // Set focus to input line
    m_inputLine->setFocus();
    
    // Add welcome message
    addOutput("StdGeo Command Line Interface");
    addOutput("Type 'help' for available commands");
    addOutput("");
}

void CommandLine::addOutput(const QString &text)
{
    m_outputArea->append(text);
    
    // Auto-scroll to bottom
    QScrollBar *scrollBar = m_outputArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void CommandLine::clearOutput()
{
    m_outputArea->clear();
    addOutput("StdGeo Command Line Interface");
    addOutput("Type 'help' for available commands");
    addOutput("");
}

void CommandLine::executeCommand()
{
    QString command = m_inputLine->text().trimmed();
    if (command.isEmpty()) {
        return;
    }
    
    // Add to history
    m_commandHistory.prepend(command);
    if (m_commandHistory.size() > 100) {
        m_commandHistory.removeLast();
    }
    m_historyIndex = -1;
    
    // Display command in output
    addOutput(QString("> %1").arg(command));
    
    // Clear input
    m_inputLine->clear();
    
    // Emit signal for processing
    emit commandExecuted(command);
}

void CommandLine::onReturnPressed()
{
    executeCommand();
}

void CommandLine::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        // Navigate up in history
        if (m_historyIndex + 1 < m_commandHistory.size()) {
            m_historyIndex++;
            m_inputLine->setText(m_commandHistory[m_historyIndex]);
        }
        event->accept();
    } else if (event->key() == Qt::Key_Down) {
        // Navigate down in history
        if (m_historyIndex > 0) {
            m_historyIndex--;
            m_inputLine->setText(m_commandHistory[m_historyIndex]);
        } else if (m_historyIndex == 0) {
            m_historyIndex = -1;
            m_inputLine->clear();
        }
        event->accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}

