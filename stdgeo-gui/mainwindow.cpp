#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_openglWidget(nullptr)
    , m_textEdit(nullptr)
    , m_splitter(nullptr)
{
    setWindowTitle("StdGeo OpenGL Viewer");
    setMinimumSize(800, 600);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    m_openglWidget = new OpenGLWidget(this);
    m_openglWidget->setMinimumSize(400, 300);
    
    m_textEdit = new QTextEdit(this);
    m_textEdit->setMinimumSize(200, 300);
    m_textEdit->setPlainText("OpenGL Viewer Text Window\n\nControls:\n- Left mouse drag: Rotate\n- Right mouse drag: Pan\n- Mouse wheel: Zoom\n\nThis text area can be used for reading or writing text.");
    
    m_splitter->addWidget(m_openglWidget);
    m_splitter->addWidget(m_textEdit);
    
    m_splitter->setSizes({600, 200});
    
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(m_splitter);
    layout->setContentsMargins(5, 5, 5, 5);
}

MainWindow::~MainWindow()
{
}