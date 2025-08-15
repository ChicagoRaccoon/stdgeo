#include <QCoreApplication>
#include <QTextStream>
#include <QCommandLineParser>
#include <iostream>
#include <string>
#include "stdgeo.h"

class HeadlessProcessor {
public:
    HeadlessProcessor() = default;
    
    void processCommand(const QString& command) {
        QTextStream out(stdout);
        QString cmd = command.trimmed();
        
        // Skip empty lines and comments
        if (cmd.isEmpty() || cmd.startsWith("#")) {
            return;
        }
        
        cmd = cmd.toLower();
        
        if (cmd == "help") {
            showHelp();
        } else if (cmd == "cube") {
            createCube(2.0);
        } else if (cmd.startsWith("cube ")) {
            QStringList parts = command.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                bool ok;
                double size = parts[1].toDouble(&ok);
                if (ok && size > 0) {
                    createCube(size);
                } else {
                    out << "Error: Invalid cube size" << Qt::endl;
                }
            }
        } else if (cmd == "stats") {
            showStats();
        } else if (cmd == "clear") {
            clearMesh();
        } else if (cmd == "exit" || cmd == "quit") {
            QCoreApplication::quit();
        } else if (!cmd.isEmpty()) {
            out << "Unknown command: " << command << Qt::endl;
            out << "Type 'help' for available commands" << Qt::endl;
        }
    }
    
    void showHelp() {
        QTextStream out(stdout);
        out << "StdGeo Headless Mode - Available Commands:" << Qt::endl;
        out << "  cube [size]  - Create a cube (default size: 2.0)" << Qt::endl;
        out << "  clear        - Clear current geometry" << Qt::endl;
        out << "  stats        - Show current geometry statistics" << Qt::endl;
        out << "  help         - Show this help message" << Qt::endl;
        out << "  exit/quit    - Exit the application" << Qt::endl;
    }
    
    void createCube(double size) {
        clearMesh();
        m_currentMesh = mesh_create_cube(size);
        
        QTextStream out(stdout);
        if (m_currentMesh) {
            size_t vertices = mesh_vertex_count(m_currentMesh);
            size_t triangles = mesh_triangle_count(m_currentMesh);
            out << "Created cube (size: " << size << ") with " 
                << vertices << " vertices and " << triangles << " triangles" << Qt::endl;
        } else {
            out << "Failed to create cube" << Qt::endl;
        }
    }
    
    void clearMesh() {
        if (m_currentMesh) {
            mesh_free(m_currentMesh);
            m_currentMesh = nullptr;
            QTextStream out(stdout);
            out << "Cleared geometry" << Qt::endl;
        }
    }
    
    void showStats() {
        QTextStream out(stdout);
        if (m_currentMesh) {
            size_t vertices = mesh_vertex_count(m_currentMesh);
            size_t triangles = mesh_triangle_count(m_currentMesh);
            out << "Current geometry: " << vertices << " vertices, " 
                << triangles << " triangles" << Qt::endl;
            
            // Show first few vertices as example
            if (vertices > 0) {
                out << "Sample vertices:" << Qt::endl;
                for (size_t i = 0; i < std::min(vertices, (size_t)3); ++i) {
                    double x, y, z;
                    if (mesh_get_vertex(m_currentMesh, i, &x, &y, &z)) {
                        out << "  [" << i << "]: (" << x << ", " << y << ", " << z << ")" << Qt::endl;
                    }
                }
            }
        } else {
            out << "No geometry loaded" << Qt::endl;
        }
    }
    
    ~HeadlessProcessor() {
        if (m_currentMesh) {
            mesh_free(m_currentMesh);
        }
    }
    
private:
    Mesh* m_currentMesh = nullptr;
};

int runHeadless(QCoreApplication& app) {
    QTextStream out(stdout);
    QTextStream in(stdin);
    
    out << "StdGeo Headless Mode" << Qt::endl;
    out << "Type 'help' for commands, 'exit' to quit" << Qt::endl;
    
    HeadlessProcessor processor;
    processor.showHelp();
    
    while (true) {
        out << "> ";
        out.flush();
        
        QString line = in.readLine();
        if (line.isNull()) {
            // EOF reached
            break;
        }
        
        processor.processCommand(line);
        
        // Process any pending Qt events
        app.processEvents();
    }
    
    return 0;
}