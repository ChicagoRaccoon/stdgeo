#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class CommandLine : public QWidget
{
    Q_OBJECT

public:
    explicit CommandLine(QWidget *parent = nullptr);
    
    void addOutput(const QString &text);
    void clearOutput();

signals:
    void commandExecuted(const QString &command);

private slots:
    void executeCommand();
    void onReturnPressed();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupUI();
    
    QTextEdit *m_outputArea;
    QLineEdit *m_inputLine;
    QPushButton *m_executeButton;
    QPushButton *m_clearButton;
    
    QStringList m_commandHistory;
    int m_historyIndex;
};

#endif // COMMANDLINE_H