/**
 * @file mainwindow.h
 * @brief Main application window for the StdGeo Qt GUI viewer
 * 
 * This file defines the MainWindow class, which provides the main application window
 * containing an interactive 3D OpenGL viewer and a text editor panel arranged in
 * a resizable horizontal splitter layout.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt GUI framework includes
#include <QMainWindow>   // Base class for main application windows
#include <QTextEdit>     // Multi-line text editor widget
#include <QSplitter>     // Resizable pane splitter widget

// Application-specific includes
#include "openglwidget.h"  // Custom OpenGL 3D viewer widget

/**
 * @class MainWindow
 * @brief Main application window class
 * 
 * The MainWindow provides the primary user interface for the StdGeo Qt GUI application.
 * It contains two main components arranged horizontally:
 * - Top pane: Interactive 3D OpenGL viewer displaying a colorful cube
 * - Bottom pane: Multi-line text editor for user notes or data
 * 
 * The panes are separated by a draggable splitter that allows users to adjust
 * the relative sizes of the OpenGL viewer and text editor areas.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget (optional, typically nullptr for main window)
     * 
     * Creates and initializes the main window with its child widgets:
     * - Sets up the OpenGL viewer widget
     * - Creates the text editor with default content
     * - Configures the vertical splitter layout
     * - Sets window title and minimum size
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up resources and child widgets. Qt's parent-child ownership
     * model handles most cleanup automatically.
     */
    ~MainWindow();

private:
    // UI component pointers - managed by Qt's parent-child ownership
    
    OpenGLWidget *m_openglWidget;  ///< Interactive 3D OpenGL viewer (left pane)
    QTextEdit *m_textEdit;         ///< Multi-line text editor (right pane)
    QSplitter *m_splitter;         ///< Vertical splitter managing the two panes
};

#endif // MAINWINDOW_H