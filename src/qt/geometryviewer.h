#ifndef GEOMETRYVIEWER_H
#define GEOMETRYVIEWER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QWheelEvent>
#include "stdgeo.h"

class GeometryViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GeometryViewer(QWidget *parent = nullptr);
    ~GeometryViewer();

    void clearMesh();
    void createCube(double size);
    void setMesh(Mesh* mesh);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupGeometry();
    void updateMesh();
    void setupShaders();
    void setupCamera();
    
    // OpenGL objects
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_indexBuffer;
    QOpenGLVertexArrayObject m_vao;
    
    // Matrices
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    
    // Camera control
    float m_cameraDistance;
    float m_cameraRotationX;
    float m_cameraRotationY;
    QPoint m_lastMousePos;
    bool m_mousePressed;
    
    // Geometry data
    Mesh *m_currentMesh;
    QVector<float> m_vertices;
    QVector<unsigned int> m_indices;
    bool m_meshNeedsUpdate;
};

#endif // GEOMETRYVIEWER_H