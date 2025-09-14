#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QSplitter>
#include "openglwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    OpenGLWidget *m_openglWidget;
    QTextEdit *m_textEdit;
    QSplitter *m_splitter;
};

#endif // MAINWINDOW_H