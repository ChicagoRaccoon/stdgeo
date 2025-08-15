#include "mainwindow.h"
#include "geometryviewer.h"
#include "commandline.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_viewer(nullptr)
    , m_commandLine(nullptr)
    , m_splitter(nullptr)
{
    setWindowTitle("StdGeo Viewer");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    setupMenus();
    setupToolbars();
    setupStatusBar();
    setupCentralWidget();
    
    // Load settings
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
    // Save settings
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    m_newAction = new QAction("&New", this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("Create a new geometry");
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newMesh);
    fileMenu->addAction(m_newAction);
    
    m_openAction = new QAction("&Open...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("Open an existing geometry file");
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openMesh);
    fileMenu->addAction(m_openAction);
    
    m_saveAction = new QAction("&Save...", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("Save the current geometry");
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveMesh);
    fileMenu->addAction(m_saveAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(m_exitAction);
    
    // Geometry menu
    QMenu *geometryMenu = menuBar()->addMenu("&Geometry");
    
    m_createCubeAction = new QAction("Create &Cube", this);
    m_createCubeAction->setStatusTip("Create a cube geometry");
    connect(m_createCubeAction, &QAction::triggered, this, &MainWindow::createCube);
    geometryMenu->addAction(m_createCubeAction);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    m_aboutAction = new QAction("&About", this);
    m_aboutAction->setStatusTip("Show information about this application");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupToolbars()
{
    // File toolbar
    m_fileToolBar = addToolBar("File");
    m_fileToolBar->addAction(m_newAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);
    
    // Geometry toolbar
    m_geometryToolBar = addToolBar("Geometry");
    m_geometryToolBar->addAction(m_createCubeAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::setupCentralWidget()
{
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    // Create geometry viewer
    m_viewer = new GeometryViewer(this);
    m_splitter->addWidget(m_viewer);
    
    // Create command line
    m_commandLine = new CommandLine(this);
    m_splitter->addWidget(m_commandLine);
    
    // Set initial sizes (3/4 for viewer, 1/4 for command line)
    m_splitter->setSizes({600, 200});
    
    setCentralWidget(m_splitter);
    
    // Connect command line to processing
    connect(m_commandLine, &CommandLine::commandExecuted, 
            this, &MainWindow::onCommandExecuted);
}

void MainWindow::newMesh()
{
    if (m_viewer) {
        m_viewer->clearMesh();
        statusBar()->showMessage("New geometry created", 2000);
    }
}

void MainWindow::openMesh()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Geometry File", "", "All Files (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: Implement file loading
        statusBar()->showMessage("File loading not yet implemented", 2000);
    }
}

void MainWindow::saveMesh()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Geometry File", "", "All Files (*)");
    
    if (!fileName.isEmpty()) {
        // TODO: Implement file saving
        statusBar()->showMessage("File saving not yet implemented", 2000);
    }
}

void MainWindow::createCube()
{
    if (m_viewer) {
        m_viewer->createCube(2.0);
        statusBar()->showMessage("Cube created", 2000);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, "About StdGeo Viewer",
        "StdGeo Viewer 1.0.0\n\n"
        "A 3D geometry viewer and manipulator\n"
        "Built with Qt and Rust\n\n"
        "Features:\n"
        "• 3D geometry viewing\n"
        "• Command line interface\n"
        "• Extensible toolbar system");
}

void MainWindow::onCommandExecuted(const QString &command)
{
    // Process commands from the command line
    QString cmd = command.trimmed().toLower();
    
    if (cmd == "cube") {
        createCube();
        m_commandLine->addOutput("Created cube with size 2.0");
    } else if (cmd == "clear") {
        newMesh();
        m_commandLine->addOutput("Cleared geometry");
    } else if (cmd.startsWith("cube ")) {
        // Parse cube size
        QStringList parts = command.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            bool ok;
            double size = parts[1].toDouble(&ok);
            if (ok && size > 0) {
                m_viewer->createCube(size);
                m_commandLine->addOutput(QString("Created cube with size %1").arg(size));
            } else {
                m_commandLine->addOutput("Error: Invalid cube size");
            }
        }
    } else if (cmd == "help") {
        m_commandLine->addOutput("Available commands:");
        m_commandLine->addOutput("  cube [size] - Create a cube");
        m_commandLine->addOutput("  clear       - Clear geometry");
        m_commandLine->addOutput("  help        - Show this help");
    } else {
        m_commandLine->addOutput(QString("Unknown command: %1").arg(command));
        m_commandLine->addOutput("Type 'help' for available commands");
    }
    
    statusBar()->showMessage(QString("Executed: %1").arg(command), 2000);
}

