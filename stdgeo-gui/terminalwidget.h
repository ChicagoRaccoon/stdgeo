#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QTimer>
#include <QScrollBar>
#include <QKeyEvent>
#include <QFont>
#include <QFileSystemWatcher>
#include "stdgeoparser.h"

// Using stdgeo-parser library for direct command execution

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    explicit TerminalWidget(StdGeoParser *sharedParser, QWidget *parent = nullptr);
    ~TerminalWidget();

    void executeCommand(const QString &command);
    void clear();
    bool isParserReady() const;

public slots:
    void startInteractiveSession();
    void stopSession();
    void sendCommand(const QString &command);

signals:
    void commandExecuted(const QString &command, const QString &output);
    void sessionStarted();
    void sessionEnded();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCommandLineReturnPressed();
    void onClearButtonClicked();
    void onSessionButtonClicked();
    void watchForGeometryFiles();
    void onCommandExecuted(const QString &command, const QString &result, bool success);

private:
    void setupUI();
    void setupParser();
    void setupSharedParser();
    void appendOutput(const QString &text, const QColor &color = Qt::black);
    void appendPrompt();
    void updateSessionButton();
    void setupFileWatcher();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QTextEdit *m_outputDisplay;
    QLineEdit *m_commandLine;
    QPushButton *m_sessionButton;
    QPushButton *m_clearButton;
    QLabel *m_statusLabel;
    

    // Parser management
    StdGeoParser *m_parser;
    bool m_sessionMode;
    QTimer *m_outputTimer;
    
    // File watching for real-time updates
    QFileSystemWatcher *m_fileWatcher;
    QStringList m_watchedFiles;
    
    // Command history
    QStringList m_commandHistory;
    int m_historyIndex;
    
    // Colors and styling
    QColor m_outputColor;
    QColor m_errorColor;
    QColor m_promptColor;
    QColor m_commandColor;
    QFont m_terminalFont;
};

#endif // TERMINALWIDGET_H