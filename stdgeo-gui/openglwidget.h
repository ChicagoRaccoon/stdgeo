/**
 * @file openglwidget.h
 * @brief OpenGL widget for interactive 3D rendering
 * 
 * This file defines the OpenGLWidget class, which provides an interactive 3D viewer
 * that displays a colorful cube with mouse-based controls for rotation, panning, and zooming.
 */

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

// Qt OpenGL and graphics includes
#include <QOpenGLWidget>      // Base class for OpenGL rendering in Qt
#include <QOpenGLFunctions>   // OpenGL function access
#include <QMatrix4x4>         // 4x4 transformation matrices
#include <QVector3D>          // 3D vector operations

// Qt event handling includes
#include <QMouseEvent>        // Mouse event handling
#include <QWheelEvent>        // Mouse wheel event handling

/**
 * @class OpenGLWidget
 * @brief Interactive 3D OpenGL viewer widget
 * 
 * This widget renders a 3D scene containing a colorful cube that can be interactively
 * manipulated using mouse controls:
 * - Left mouse drag: Rotate the cube
 * - Right mouse drag: Pan the view
 * - Mouse wheel: Zoom in/out
 * 
 * The widget inherits from QOpenGLWidget for Qt integration and QOpenGLFunctions
 * for direct OpenGL API access.
 */
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget (optional)
     */
    explicit OpenGLWidget(QWidget *parent = nullptr);

protected:
    // OpenGL lifecycle methods - called automatically by Qt
    
    /**
     * @brief Initialize OpenGL context and settings
     * Called once when the OpenGL context is first created
     */
    void initializeGL() override;
    
    /**
     * @brief Render the 3D scene
     * Called whenever the widget needs to be repainted
     */
    void paintGL() override;
    
    /**
     * @brief Handle viewport resizing
     * @param width New viewport width in pixels
     * @param height New viewport height in pixels
     * Called when the widget is resized
     */
    void resizeGL(int width, int height) override;
    
    // Mouse interaction event handlers
    
    /**
     * @brief Handle mouse button press events
     * @param event Mouse press event details
     * Tracks mouse position for drag operations
     */
    void mousePressEvent(QMouseEvent *event) override;
    
    /**
     * @brief Handle mouse movement events
     * @param event Mouse move event details
     * Implements rotation (left button) and panning (right button)
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    
    /**
     * @brief Handle mouse wheel scroll events
     * @param event Wheel event details
     * Implements zoom in/out functionality
     */
    void wheelEvent(QWheelEvent *event) override;

private:
    /**
     * @brief Render the 3D cube using immediate mode OpenGL
     * Draws a cube with different colored faces using GL_QUADS
     */
    void drawCube();
    
    // Transformation matrices for 3D rendering
    QMatrix4x4 m_projection;   ///< Projection matrix (perspective)
    QMatrix4x4 m_view;         ///< View matrix (camera)  
    QMatrix4x4 m_model;        ///< Model matrix (object transform)
    
    // Interactive transformation parameters
    QVector3D m_rotation;      ///< Current rotation angles (x, y, z)
    QVector3D m_translation;   ///< Current translation offset
    float m_scale;             ///< Current scale factor for zoom
    
    // Mouse interaction state
    QPoint m_lastMousePos;     ///< Last recorded mouse position
    bool m_mousePressed;       ///< Whether any mouse button is currently pressed
};

#endif // OPENGLWIDGET_H