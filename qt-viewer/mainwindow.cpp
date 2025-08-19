#include "mainwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFileInfo>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralSplitter(nullptr)
    , m_geometryViewer(nullptr)
    , m_terminalWidget(nullptr)
    , m_geometryCollection(nullptr)
    , m_refreshTimer(new QTimer(this))
{
    // Initialize geometry collection
    m_geometryCollection = geometry_collection_new();
    
    // Setup UI
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    setupLayout();
    connectSignals();
    
    // Set window properties
    setWindowTitle("StdGeo Viewer");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Setup refresh timer for real-time updates from terminal
    m_refreshTimer->setInterval(100); // 10 FPS refresh rate
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshView);
    m_refreshTimer->start();
    
    // Initial view setup
    resetView();
}

MainWindow::~MainWindow()
{
    if (m_geometryCollection) {
        geometry_collection_free(m_geometryCollection);
    }
}

void MainWindow::createActions()
{
    // File actions
    m_newAction = new QAction(tr("&New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new geometry collection"));
    
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open a geometry file"));
    
    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current geometry collection"));
    
    m_saveAsAction = new QAction(tr("Save &As..."), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the geometry collection with a new name"));
    
    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    
    // View actions
    m_resetViewAction = new QAction(tr("&Reset View"), this);
    m_resetViewAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    m_resetViewAction->setStatusTip(tr("Reset the view to default"));
    
    m_fitToWindowAction = new QAction(tr("&Fit to Window"), this);
    m_fitToWindowAction->setShortcut(QKeySequence(tr("Ctrl+F")));
    m_fitToWindowAction->setStatusTip(tr("Fit all geometry to the window"));
    
    m_refreshAction = new QAction(tr("Refresh"), this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    m_refreshAction->setStatusTip(tr("Refresh the geometry display"));
    
    // Help actions
    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show information about this application"));
    
    m_aboutQtAction = new QAction(tr("About &Qt"), this);
    m_aboutQtAction->setStatusTip(tr("Show information about Qt"));
}

void MainWindow::createMenus()
{
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);
    
    // View menu
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_resetViewAction);
    m_viewMenu->addAction(m_fitToWindowAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_refreshAction);
    
    // Help menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::createToolBars()
{
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);
    
    // View toolbar
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->addAction(m_resetViewAction);
    m_viewToolBar->addAction(m_fitToWindowAction);
    m_viewToolBar->addAction(m_refreshAction);
}

void MainWindow::createStatusBar()
{
    m_geometryCountLabel = new QLabel(tr("Geometries: 0"));
    m_coordinateLabel = new QLabel(tr("Coordinates: (0, 0)"));
    
    statusBar()->addWidget(m_geometryCountLabel);
    statusBar()->addPermanentWidget(m_coordinateLabel);
    
    statusBar()->showMessage(tr("Ready"), 2000);
}

void MainWindow::setupLayout()
{
    // Create central splitter
    m_centralSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_centralSplitter);
    
    // Create geometry viewer
    m_geometryViewer = new GeometryViewer(this);
    m_geometryViewer->setGeometryCollection(m_geometryCollection);
    
    // Create terminal widget
    m_terminalWidget = new TerminalWidget(this);
    
    // Add widgets to splitter
    m_centralSplitter->addWidget(m_geometryViewer);
    m_centralSplitter->addWidget(m_terminalWidget);
    
    // Set initial splitter sizes (70% viewer, 30% terminal)
    m_centralSplitter->setSizes({700, 300});
    m_centralSplitter->setStretchFactor(0, 1);
    m_centralSplitter->setStretchFactor(1, 0);
}

void MainWindow::connectSignals()
{
    // File actions
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    
    // View actions
    connect(m_resetViewAction, &QAction::triggered, this, &MainWindow::resetView);
    connect(m_fitToWindowAction, &QAction::triggered, this, &MainWindow::fitToWindow);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshView);
    
    // Help actions
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
    connect(m_aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);
    
    // Geometry viewer signals
    connect(m_geometryViewer, &GeometryViewer::mousePositionChanged,
            [this](double x, double y) {
                m_coordinateLabel->setText(tr("Coordinates: (%1, %2)")
                    .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
            });
}

void MainWindow::newFile()
{
    geometry_collection_clear(m_geometryCollection);
    m_currentFile.clear();
    setWindowTitle("StdGeo Viewer");
    updateGeometryDisplay();
    statusBar()->showMessage(tr("New file created"), 2000);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Geometry File"), QString(),
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        geometry_collection_clear(m_geometryCollection);
        
        if (geometry_collection_load_json(m_geometryCollection, fileName.toLocal8Bit().constData()) == 0) {
            m_currentFile = fileName;
            setWindowTitle(tr("StdGeo Viewer - %1").arg(QFileInfo(fileName).fileName()));
            updateGeometryDisplay();
            fitToWindow();
            statusBar()->showMessage(tr("File loaded: %1").arg(fileName), 2000);
        } else {
            QMessageBox::warning(this, tr("Error"), 
                tr("Could not load file: %1").arg(fileName));
        }
    }
}

void MainWindow::saveFile()
{
    if (m_currentFile.isEmpty()) {
        saveAsFile();
    } else {
        if (geometry_collection_save_json(m_geometryCollection, 
                m_currentFile.toLocal8Bit().constData()) == 0) {
            statusBar()->showMessage(tr("File saved: %1").arg(m_currentFile), 2000);
        } else {
            QMessageBox::warning(this, tr("Error"), 
                tr("Could not save file: %1").arg(m_currentFile));
        }
    }
}

void MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Geometry File"), QString(),
        tr("JSON Files (*.json);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        if (geometry_collection_save_json(m_geometryCollection, 
                fileName.toLocal8Bit().constData()) == 0) {
            m_currentFile = fileName;
            setWindowTitle(tr("StdGeo Viewer - %1").arg(QFileInfo(fileName).fileName()));
            statusBar()->showMessage(tr("File saved: %1").arg(fileName), 2000);
        } else {
            QMessageBox::warning(this, tr("Error"), 
                tr("Could not save file: %1").arg(fileName));
        }
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About StdGeo Viewer"),
        tr("<h2>StdGeo Viewer 0.1.0</h2>"
           "<p>A Qt-based viewer for 2D geometry with integrated CLI.</p>"
           "<p>Features:</p>"
           "<ul>"
           "<li>Interactive 2D geometry visualization</li>"
           "<li>Mouse controls for zoom, pan, and rotate</li>"
           "<li>Integrated terminal for CLI access</li>"
           "<li>Real-time geometry updates</li>"
           "</ul>"
           "<p>Built with Qt6 and Rust.</p>"));
}

void MainWindow::aboutQt()
{
    QApplication::aboutQt();
}

void MainWindow::exitApplication()
{
    close();
}

void MainWindow::resetView()
{
    if (m_geometryViewer) {
        m_geometryViewer->resetView();
    }
}

void MainWindow::fitToWindow()
{
    if (m_geometryViewer) {
        m_geometryViewer->fitToWindow();
    }
}

void MainWindow::refreshView()
{
    updateGeometryDisplay();
    if (m_geometryViewer) {
        m_geometryViewer->update();
    }
}

void MainWindow::updateGeometryDisplay()
{
    int count = geometry_collection_size(m_geometryCollection);
    m_geometryCountLabel->setText(tr("Geometries: %1").arg(count));
    onGeometryCountChanged(count);
}

void MainWindow::onGeometryCountChanged(int count)
{
    Q_UNUSED(count)
    if (m_geometryViewer) {
        m_geometryViewer->update();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Clean shutdown
    event->accept();
}