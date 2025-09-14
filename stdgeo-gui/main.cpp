/**
 * @file main.cpp
 * @brief Application entry point for the StdGeo Qt GUI viewer
 * 
 * This file contains the main() function that initializes the Qt application
 * and creates the main window containing an interactive 3D OpenGL viewer
 * and text editor.
 * 
 * The application provides:
 * - Interactive 3D cube with mouse controls (rotate, pan, zoom)
 * - Multi-line text editor for notes or data
 * - Resizable horizontal splitter layout
 */

#include <QApplication>   // Qt application framework
#include "mainwindow.h"   // Main window implementation

/**
 * @brief Application entry point
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Application exit code (0 = success, non-zero = error)
 * 
 * Standard Qt application startup sequence:
 * 1. Create QApplication instance to manage application lifecycle
 * 2. Create and show the main window
 * 3. Enter the Qt event loop to handle user interactions
 * 4. Return the application exit code when the user closes the window
 */
int main(int argc, char *argv[])
{
    // Create Qt application instance - manages event loop, resources, settings
    QApplication app(argc, argv);

    // Create the main application window
    MainWindow window;
    
    // Make the window visible to the user
    window.show();

    // Enter the Qt event loop - handles user input, rendering, etc.
    // This call blocks until the user closes the application
    return app.exec();
}