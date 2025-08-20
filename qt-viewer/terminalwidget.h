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

// Removed qtermwidget dependency - using QProcess-based thin wrapper

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    explicit TerminalWidget(QProcess *sharedSession, QWidget *parent = nullptr);
    ~TerminalWidget();

    void executeCommand(const QString &command);
    void clear();
    bool isCliRunning() const;

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
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onCommandLineReturnPressed();
    void onClearButtonClicked();
    void onSessionButtonClicked();
    void watchForGeometryFiles();

private:
    void setupUI();
    void setupTerminal();
    void setupSharedTerminal();
    void appendOutput(const QString &text, const QColor &color = Qt::black);
    void appendPrompt();
    void updateSessionButton();
    QString findStdgeoBinary();
    void setupFileWatcher();

    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QTextEdit *m_outputDisplay;
    QLineEdit *m_commandLine;
    QPushButton *m_sessionButton;
    QPushButton *m_clearButton;
    QLabel *m_statusLabel;
    

    // Process management
    QProcess *m_cliProcess;
    bool m_sessionMode;
    QString m_stdgeoBinaryPath;
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