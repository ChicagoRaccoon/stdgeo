/*
 * Theory of Operation:
 * This is the main entry point for the StdGeo Qt Viewer application.
 * It initializes the Qt application framework, sets up application metadata,
 * applies a modern UI style if available, creates the main window,
 * and enters the Qt event loop.
 */

#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"

// Application entry point - initializes Qt framework and main window
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("StdGeo Viewer");
    app.setApplicationVersion("0.1.0");
    app.setApplicationDisplayName("StdGeo Geometry Viewer");
    app.setOrganizationName("StdGeo");
    
    // Set a modern style if available
    QStringList availableStyles = QStyleFactory::keys();
    if (availableStyles.contains("Fusion", Qt::CaseInsensitive)) {
        app.setStyle("Fusion");
    }
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}