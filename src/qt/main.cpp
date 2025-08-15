#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "headless.h"

int main(int argc, char *argv[])
{
    // Check for headless mode before creating QApplication
    bool headlessMode = false;
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--headless" || QString(argv[i]) == "-h") {
            headlessMode = true;
            break;
        }
    }
    
    if (headlessMode) {
        // Use QCoreApplication for headless mode (no GUI)
        QCoreApplication app(argc, argv);
        
        app.setApplicationName("StdGeo Viewer");
        app.setApplicationVersion("1.0.0");
        app.setOrganizationName("StdGeo");
        
        return runHeadless(app);
    } else {
        // Use QApplication for GUI mode
        QApplication app(argc, argv);
        
        app.setApplicationName("StdGeo Viewer");
        app.setApplicationVersion("1.0.0");
        app.setOrganizationName("StdGeo");
        
        // Parse command line for GUI-specific options
        QCommandLineParser parser;
        parser.setApplicationDescription("StdGeo 3D Geometry Viewer");
        parser.addHelpOption();
        parser.addVersionOption();
        
        QCommandLineOption headlessOption(QStringList() << "headless" << "h",
                                        "Run in headless mode (command line only)");
        parser.addOption(headlessOption);
        
        parser.process(app);
        
        MainWindow window;
        window.show();
        
        return app.exec();
    }
}