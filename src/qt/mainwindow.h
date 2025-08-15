#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QAction>

class GeometryViewer;
class CommandLine;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newMesh();
    void openMesh();
    void saveMesh();
    void createCube();
    void about();
    void onCommandExecuted(const QString &command);

private:
    void setupMenus();
    void setupToolbars();
    void setupStatusBar();
    void setupCentralWidget();
    
    // UI components
    GeometryViewer *m_viewer;
    CommandLine *m_commandLine;
    QSplitter *m_splitter;
    
    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_exitAction;
    QAction *m_createCubeAction;
    QAction *m_aboutAction;
    
    // Toolbars
    QToolBar *m_fileToolBar;
    QToolBar *m_geometryToolBar;
};

#endif // MAINWINDOW_H