#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <memory>

#include "geometryviewer.h"
#include "terminalwidget.h"
#include "../stdgeo_lib.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFile();
    void saveFile();
    void saveAsFile();
    void newFile();
    void about();
    void aboutQt();
    void exitApplication();
    void resetView();
    void fitToWindow();
    void refreshView();
    void onGeometryCountChanged(int count);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void setupLayout();
    void connectSignals();
    void updateGeometryDisplay();

    // UI Components
    QSplitter *m_centralSplitter;
    GeometryViewer *m_geometryViewer;
    TerminalWidget *m_terminalWidget;
    
    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
    QAction *m_resetViewAction;
    QAction *m_fitToWindowAction;
    QAction *m_refreshAction;
    
    // Menus
    QMenu *m_fileMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;
    
    // Toolbars
    QToolBar *m_fileToolBar;
    QToolBar *m_viewToolBar;
    
    // Status bar
    QLabel *m_geometryCountLabel;
    QLabel *m_coordinateLabel;
    
    // Geometry management
    GeometryCollection *m_geometryCollection;
    QString m_currentFile;
    QTimer *m_refreshTimer;
};

#endif // MAINWINDOW_H