/**
 * @file mainwindow.cpp
 * @brief Implementation of the main application window
 * 
 * This file implements the MainWindow class, which provides the main user interface
 * for the StdGeo Qt GUI application. The window contains an interactive 3D OpenGL viewer
 * and a text editor arranged in a resizable horizontal splitter layout.
 */

#include "mainwindow.h"
#include <QVBoxLayout>   // Vertical layout manager
#include <QHBoxLayout>   // Horizontal layout manager  
#include <QWidget>       // Base widget class

/**
 * @brief Constructor - Create and configure the main application window
 * @param parent Parent widget (typically nullptr for main windows)
 * 
 * Sets up the complete user interface including:
 * - Window title and minimum size
 * - Central widget with horizontal splitter layout
 * - OpenGL viewer widget (left pane) with interactive 3D cube
 * - Text editor widget (right pane) with usage instructions
 * - Proper size constraints and proportions
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_openglWidget(nullptr)   // Initialize all widget pointers to nullptr
    , m_textEdit(nullptr)       // for safety during construction
    , m_splitter(nullptr)
{
    // Configure main window properties
    setWindowTitle("StdGeo OpenGL Viewer");
    setMinimumSize(800, 600);  // Ensure adequate space for both panes
    
    // Create central widget (required by QMainWindow architecture)
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create vertical splitter for resizable panes
    m_splitter = new QSplitter(Qt::Vertical, this);
    
    // Create and configure OpenGL viewer widget (top pane)
    m_openglWidget = new OpenGLWidget(this);
    m_openglWidget->setMinimumSize(400, 300);
    
    // Create and configure text editor widget (bottom pane)
    m_textEdit = new QTextEdit(this);
    m_textEdit->setMinimumSize(400, 150);
    
    // Set helpful default text with usage instructions
    m_textEdit->setPlainText(
        "OpenGL Viewer Text Window\n\n"
        "Controls:\n"
        "- Left mouse drag: Rotate\n"
        "- Right mouse drag: Pan\n"
        "- Mouse wheel: Zoom\n\n"
        "This text area can be used for reading or writing text."
    );
    
    // Add both widgets to the splitter
    m_splitter->addWidget(m_openglWidget);  // Top pane
    m_splitter->addWidget(m_textEdit);      // Bottom pane
    
    // Set initial size proportions: 450px for OpenGL, 150px for text
    m_splitter->setSizes({450, 150});
    
    // Create layout and add splitter to central widget
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(m_splitter);
    layout->setContentsMargins(5, 5, 5, 5);  // Small margin around the edges
}

/**
 * @brief Destructor - Clean up resources
 * 
 * Qt's parent-child ownership model automatically handles cleanup of child widgets,
 * so explicit deletion is not required. This destructor is provided for completeness
 * and future resource management if needed.
 */
MainWindow::~MainWindow()
{
    // Explicit cleanup not required due to Qt's parent-child ownership model
    // Child widgets (m_openglWidget, m_textEdit, m_splitter) are automatically
    // deleted when this MainWindow is destroyed
}