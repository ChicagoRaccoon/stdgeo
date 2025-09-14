/**
 * @file openglwidget.cpp
 * @brief Implementation of the interactive 3D OpenGL viewer widget
 * 
 * This file implements the OpenGLWidget class, providing an interactive 3D viewer
 * that renders a colorful cube with mouse-based controls for rotation, panning, and zooming.
 */

#include "openglwidget.h"
#include <QOpenGLFunctions>  // OpenGL function access
#include <cmath>             // Mathematical functions

/**
 * @brief Constructor - Initialize the OpenGL widget with default transform values
 * @param parent Parent widget (typically the main window)
 * 
 * Sets up initial transformation state:
 * - No rotation (0, 0, 0)
 * - Positioned back from camera (-5 units in Z)
 * - Normal scale (1.0)
 * - Mouse interaction disabled initially
 */
OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rotation(0.0f, 0.0f, 0.0f)      // No initial rotation
    , m_translation(0.0f, 0.0f, -5.0f)  // Move cube back from camera
    , m_scale(1.0f)                     // Normal scale
    , m_mousePressed(false)             // No mouse interaction initially
{
}

/**
 * @brief Initialize OpenGL context and rendering settings
 * 
 * Called automatically by Qt when the OpenGL context is first created.
 * Sets up:
 * - OpenGL function pointers for this context
 * - Depth testing for proper 3D rendering
 * - Background clear color (dark gray)
 */
void OpenGLWidget::initializeGL()
{
    // Initialize OpenGL function pointers for this context
    initializeOpenGLFunctions();
    
    // Enable depth testing for proper 3D object rendering
    glEnable(GL_DEPTH_TEST);
    
    // Set background color to dark gray (R=0.2, G=0.2, B=0.2, A=1.0)
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

/**
 * @brief Render the 3D scene
 * 
 * Called automatically by Qt whenever the widget needs repainting.
 * Performs the complete rendering pipeline:
 * 1. Clear color and depth buffers
 * 2. Set up model transformation matrix (translation, scale, rotation)
 * 3. Compute model-view-projection matrix
 * 4. Apply transformation to OpenGL
 * 5. Render the cube geometry
 */
void OpenGLWidget::paintGL()
{
    // Clear both color and depth buffers for new frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Build model transformation matrix from user interactions
    m_model.setToIdentity();                             // Start with identity
    m_model.translate(m_translation);                    // Apply panning
    m_model.scale(m_scale);                              // Apply zoom scaling
    m_model.rotate(m_rotation.x(), 1.0f, 0.0f, 0.0f);    // Rotate around X-axis
    m_model.rotate(m_rotation.y(), 0.0f, 1.0f, 0.0f);    // Rotate around Y-axis  
    m_model.rotate(m_rotation.z(), 0.0f, 0.0f, 1.0f);    // Rotate around Z-axis
    
    // Compute final model-view-projection matrix
    QMatrix4x4 mvp = m_projection * m_view * m_model;
    
    // Apply transformation to OpenGL fixed-function pipeline
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mvp.constData());
    
    // Render the cube geometry
    drawCube();
}

/**
 * @brief Handle widget resizing and update projection matrix
 * @param width New widget width in pixels
 * @param height New widget height in pixels
 * 
 * Called automatically by Qt when the widget is resized.
 * Updates the OpenGL viewport and recalculates the projection matrix
 * to maintain correct aspect ratio and prevent distortion.
 */
void OpenGLWidget::resizeGL(int width, int height)
{
    // Set OpenGL viewport to match new widget size
    glViewport(0, 0, width, height);
    
    // Recalculate projection matrix with new aspect ratio
    m_projection.setToIdentity();
    float aspect = float(width) / float(height ? height : 1);  // Avoid division by zero
    m_projection.perspective(45.0f, aspect, 0.1f, 100.0f);     // 45° FOV, near=0.1, far=100
    
    // Reset view matrix (camera at origin looking down -Z)
    m_view.setToIdentity();
}

/**
 * @brief Handle mouse button press events
 * @param event Mouse press event containing button and position information
 * 
 * Records the initial mouse position for subsequent drag calculations
 * and enables mouse tracking for interactive transformations.
 */
void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    // Store current mouse position for delta calculations in mouseMoveEvent
    m_lastMousePos = event->pos();
    
    // Enable mouse interaction tracking
    m_mousePressed = true;
}

/**
 * @brief Handle mouse movement for interactive transformations
 * @param event Mouse move event containing current position and button state
 * 
 * Implements two interaction modes based on which mouse button is pressed:
 * - Left button: Rotate the cube around X and Y axes
 * - Right button: Pan (translate) the cube in the view plane
 * 
 * The transformations are applied incrementally based on mouse movement delta.
 */
void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Only process mouse movement if a button was initially pressed
    if (m_mousePressed) {
        // Calculate movement delta since last mouse position
        QPoint delta = event->pos() - m_lastMousePos;
        
        // Left mouse button: Rotate the cube
        if (event->buttons() & Qt::LeftButton) {
            // Y mouse movement -> X-axis rotation (pitch)
            m_rotation.setX(m_rotation.x() + delta.y() * 0.5f);
            // X mouse movement -> Y-axis rotation (yaw)
            m_rotation.setY(m_rotation.y() + delta.x() * 0.5f);
        }
        // Right mouse button: Pan the cube
        else if (event->buttons() & Qt::RightButton) {
            // X mouse movement -> X translation
            m_translation.setX(m_translation.x() + delta.x() * 0.01f);
            // Y mouse movement -> Y translation (inverted for natural feel)
            m_translation.setY(m_translation.y() - delta.y() * 0.01f);
        }
        
        // Update mouse position for next delta calculation
        m_lastMousePos = event->pos();
        
        // Trigger repaint to show the transformation
        update();
    }
}

/**
 * @brief Handle mouse wheel events for zoom functionality
 * @param event Wheel event containing scroll direction and magnitude
 * 
 * Implements zoom in/out by scaling the cube uniformly.
 * Scroll up = zoom in (larger scale), scroll down = zoom out (smaller scale).
 * Scale is clamped between 0.1x and 10x to prevent excessive zoom levels.
 */
void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    // Convert wheel angle delta to scale factor
    // Most mice report 120 units per "notch" of the wheel
    float delta = event->angleDelta().y() / 120.0f;
    
    // Apply proportional scaling: positive delta = zoom in, negative = zoom out
    m_scale *= (1.0f + delta * 0.1f);  // 10% scale change per wheel notch
    
    // Clamp scale to reasonable limits to prevent extreme zoom levels
    m_scale = qMax(0.1f, qMin(10.0f, m_scale));  // Min: 0.1x, Max: 10x
    
    // Trigger repaint to show the scale change
    update();
}

/**
 * @brief Render a colorful 3D cube using immediate mode OpenGL
 * 
 * Draws a unit cube (2x2x2) centered at the origin using GL_QUADS.
 * Each face is rendered with a different solid color to make rotation visible:
 * - Front face: Red
 * - Back face: Green  
 * - Top face: Blue
 * - Bottom face: Yellow
 * - Right face: Magenta
 * - Left face: Cyan
 * 
 * Uses immediate mode OpenGL (glBegin/glEnd) which is deprecated in modern
 * OpenGL but simple for demonstration purposes.
 */
void OpenGLWidget::drawCube()
{
    // Start rendering quadrilaterals (4-vertex faces)
    glBegin(GL_QUADS);
    
    // Front face (Z = +1) - Red
    glColor3f(1.0f, 0.0f, 0.0f);  // Set color to red
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom-left
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom-right
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Top-right
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Top-left
    
    // Back face (Z = -1) - Green
    glColor3f(0.0f, 1.0f, 0.0f);  // Set color to green
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom-left
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Top-left
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Top-right
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom-right
    
    // Top face (Y = +1) - Blue
    glColor3f(0.0f, 0.0f, 1.0f);  // Set color to blue
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Back-left
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Front-left
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Front-right
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Back-right
    
    // Bottom face (Y = -1) - Yellow
    glColor3f(1.0f, 1.0f, 0.0f);  // Set color to yellow
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Back-left
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Back-right
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Front-right
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Front-left
    
    // Right face (X = +1) - Magenta
    glColor3f(1.0f, 0.0f, 1.0f);  // Set color to magenta
    glVertex3f( 1.0f, -1.0f, -1.0f);  // Back-bottom
    glVertex3f( 1.0f,  1.0f, -1.0f);  // Back-top
    glVertex3f( 1.0f,  1.0f,  1.0f);  // Front-top
    glVertex3f( 1.0f, -1.0f,  1.0f);  // Front-bottom
    
    // Left face (X = -1) - Cyan
    glColor3f(0.0f, 1.0f, 1.0f);  // Set color to cyan
    glVertex3f(-1.0f, -1.0f, -1.0f);  // Back-bottom
    glVertex3f(-1.0f, -1.0f,  1.0f);  // Front-bottom
    glVertex3f(-1.0f,  1.0f,  1.0f);  // Front-top
    glVertex3f(-1.0f,  1.0f, -1.0f);  // Back-top
    
    // End quadrilateral rendering
    glEnd();
}