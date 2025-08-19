#ifndef GEOMETRYVIEWER_H
#define GEOMETRYVIEWER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include "../stdgeo_lib.h"

class GeometryViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GeometryViewer(QWidget *parent = nullptr);
    ~GeometryViewer();

    void setGeometryCollection(GeometryCollection *collection);
    void resetView();
    void fitToWindow();

signals:
    void mousePositionChanged(double x, double y);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void animate();

private:
    void setupProjection();
    void drawGeometry();
    void drawPoint(const CPoint &point);
    void drawLine(const CLine &line);
    void drawGrid();
    void drawAxes();
    
    QPointF screenToWorld(const QPoint &screenPos) const;
    QPoint worldToScreen(const QPointF &worldPos) const;
    void updateBounds();

    // Geometry data
    GeometryCollection *m_geometryCollection;
    
    // View transformation
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    
    // Camera/view state
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_zoomFactor;
    float m_rotationAngle;
    QPointF m_panOffset;
    
    // Bounds
    double m_minX, m_minY, m_maxX, m_maxY;
    bool m_boundsValid;
    
    // Mouse interaction
    bool m_mousePressed;
    QPoint m_lastMousePos;
    Qt::MouseButton m_mouseButton;
    
    // Animation
    QTimer *m_animationTimer;
    
    // Rendering options
    bool m_showGrid;
    bool m_showAxes;
    QColor m_backgroundColor;
    QColor m_pointColor;
    QColor m_lineColor;
    QColor m_gridColor;
    QColor m_axisColor;
    float m_pointSize;
    float m_lineWidth;
    
    // Constants
    static constexpr float MIN_ZOOM = 0.01f;
    static constexpr float MAX_ZOOM = 100.0f;
    static constexpr float ZOOM_SENSITIVITY = 0.1f;
    static constexpr float PAN_SENSITIVITY = 1.0f;
    static constexpr float ROTATION_SENSITIVITY = 0.5f;
};

#endif // GEOMETRYVIEWER_H