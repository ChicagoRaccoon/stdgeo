#include "openglwidget.h"
#include <QOpenGLFunctions>
#include <cmath>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_translation(0.0f, 0.0f, -5.0f)
    , m_scale(1.0f)
    , m_mousePressed(false)
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_model.setToIdentity();
    m_model.translate(m_translation);
    m_model.scale(m_scale);
    m_model.rotate(m_rotation.x(), 1.0f, 0.0f, 0.0f);
    m_model.rotate(m_rotation.y(), 0.0f, 1.0f, 0.0f);
    m_model.rotate(m_rotation.z(), 0.0f, 0.0f, 1.0f);
    
    QMatrix4x4 mvp = m_projection * m_view * m_model;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mvp.constData());
    
    drawCube();
}

void OpenGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    
    m_projection.setToIdentity();
    float aspect = float(width) / float(height ? height : 1);
    m_projection.perspective(45.0f, aspect, 0.1f, 100.0f);
    
    m_view.setToIdentity();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    m_mousePressed = true;
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed) {
        QPoint delta = event->pos() - m_lastMousePos;
        
        if (event->buttons() & Qt::LeftButton) {
            m_rotation.setX(m_rotation.x() + delta.y() * 0.5f);
            m_rotation.setY(m_rotation.y() + delta.x() * 0.5f);
        } else if (event->buttons() & Qt::RightButton) {
            m_translation.setX(m_translation.x() + delta.x() * 0.01f);
            m_translation.setY(m_translation.y() - delta.y() * 0.01f);
        }
        
        m_lastMousePos = event->pos();
        update();
    }
}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_scale *= (1.0f + delta * 0.1f);
    m_scale = qMax(0.1f, qMin(10.0f, m_scale));
    update();
}

void OpenGLWidget::drawCube()
{
    glColor3f(1.0f, 0.0f, 0.0f);
    
    glBegin(GL_QUADS);
    
    // Front face
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    
    // Back face
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    
    // Top face
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    
    // Bottom face
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    
    // Right face
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    
    // Left face
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    
    glEnd();
}