#include "geometryviewer.h"
#include <QOpenGLFunctions>
#include <QtMath>
#include <QDebug>

GeometryViewer::GeometryViewer(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_geometryCollection(nullptr)
    , m_zoomFactor(1.0f)
    , m_rotationAngle(0.0f)
    , m_panOffset(0.0f, 0.0f)
    , m_minX(0.0), m_minY(0.0), m_maxX(0.0), m_maxY(0.0)
    , m_boundsValid(false)
    , m_mousePressed(false)
    , m_mouseButton(Qt::NoButton)
    , m_animationTimer(new QTimer(this))
    , m_showGrid(true)
    , m_showAxes(true)
    , m_backgroundColor(Qt::white)
    , m_pointColor(Qt::blue)
    , m_lineColor(Qt::black)
    , m_gridColor(Qt::lightGray)
    , m_axisColor(Qt::darkGray)
    , m_pointSize(5.0f)
    , m_lineWidth(2.0f)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Setup camera
    m_cameraPosition = QVector3D(0.0f, 0.0f, 10.0f);
    m_cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    m_cameraUp = QVector3D(0.0f, 1.0f, 0.0f);
    
    // Setup animation timer
    connect(m_animationTimer, &QTimer::timeout, this, &GeometryViewer::animate);
    m_animationTimer->start(16); // ~60 FPS
}

GeometryViewer::~GeometryViewer()
{
}

void GeometryViewer::setGeometryCollection(GeometryCollection *collection)
{
    m_geometryCollection = collection;
    m_boundsValid = false;
    update();
}

void GeometryViewer::resetView()
{
    m_zoomFactor = 1.0f;
    m_rotationAngle = 0.0f;
    m_panOffset = QPointF(0.0f, 0.0f);
    update();
}

void GeometryViewer::fitToWindow()
{
    if (!m_geometryCollection) return;
    
    updateBounds();
    if (!m_boundsValid) return;
    
    // Calculate the geometry bounds
    double width = m_maxX - m_minX;
    double height = m_maxY - m_minY;
    
    if (width <= 0.0 || height <= 0.0) return;
    
    // Calculate zoom to fit both dimensions with some padding
    double aspectRatio = static_cast<double>(this->width()) / this->height();
    double geometryAspect = width / height;
    
    double zoomX = (this->width() * 0.8) / width;
    double zoomY = (this->height() * 0.8) / height;
    m_zoomFactor = qMin(zoomX, zoomY);
    
    // Center the view on the geometry
    m_panOffset.setX(-(m_minX + width / 2.0) * m_zoomFactor);
    m_panOffset.setY(-(m_minY + height / 2.0) * m_zoomFactor);
    
    update();
}

void GeometryViewer::initializeGL()
{
    initializeOpenGLFunctions();
    
    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), 
                 m_backgroundColor.blueF(), 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void GeometryViewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    setupProjection();
    
    if (m_showGrid) {
        drawGrid();
    }
    
    if (m_showAxes) {
        drawAxes();
    }
    
    drawGeometry();
}

void GeometryViewer::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    update();
}

void GeometryViewer::setupProjection()
{
    m_projectionMatrix.setToIdentity();
    
    // Use orthographic projection for 2D viewing
    float aspectRatio = static_cast<float>(width()) / height();
    float viewWidth = 10.0f / m_zoomFactor;
    float viewHeight = viewWidth / aspectRatio;
    
    m_projectionMatrix.ortho(-viewWidth/2, viewWidth/2, -viewHeight/2, viewHeight/2, -100.0f, 100.0f);
    
    // Apply pan offset
    m_projectionMatrix.translate(m_panOffset.x() / m_zoomFactor / width() * viewWidth,
                                 -m_panOffset.y() / m_zoomFactor / height() * viewHeight, 0.0f);
    
    // Apply rotation
    if (m_rotationAngle != 0.0f) {
        m_projectionMatrix.rotate(m_rotationAngle, 0.0f, 0.0f, 1.0f);
    }
}

void GeometryViewer::drawGeometry()
{
    if (!m_geometryCollection) return;
    
    int count = geometry_collection_size(m_geometryCollection);
    if (count <= 0) return;
    
    for (int i = 0; i < count; ++i) {
        CGeometry geometry;
        if (geometry_collection_get(m_geometryCollection, i, &geometry) == 0) {
            switch (geometry.geometry_type) {
                case GEOMETRY_TYPE_POINT:
                    drawPoint(geometry.data.point);
                    break;
                case GEOMETRY_TYPE_LINE:
                    drawLine(geometry.data.line);
                    break;
            }
        }
    }
}

void GeometryViewer::drawPoint(const CPoint &point)
{
    glPointSize(m_pointSize);
    glColor3f(m_pointColor.redF(), m_pointColor.greenF(), m_pointColor.blueF());
    
    glBegin(GL_POINTS);
    glVertex2d(point.x, point.y);
    glEnd();
    
    // Also draw a small circle for better visibility
    glColor3f(m_pointColor.redF(), m_pointColor.greenF(), m_pointColor.blueF());
    glBegin(GL_LINE_LOOP);
    const int segments = 12;
    const double radius = 0.1 / m_zoomFactor;
    for (int i = 0; i < segments; ++i) {
        double angle = 2.0 * M_PI * i / segments;
        glVertex2d(point.x + radius * cos(angle), point.y + radius * sin(angle));
    }
    glEnd();
}

void GeometryViewer::drawLine(const CLine &line)
{
    glLineWidth(m_lineWidth);
    glColor3f(m_lineColor.redF(), m_lineColor.greenF(), m_lineColor.blueF());
    
    glBegin(GL_LINES);
    glVertex2d(line.start.x, line.start.y);
    glVertex2d(line.end.x, line.end.y);
    glEnd();
}

void GeometryViewer::drawGrid()
{
    glColor4f(m_gridColor.redF(), m_gridColor.greenF(), m_gridColor.blueF(), 0.5f);
    glLineWidth(1.0f);
    
    // Calculate grid spacing based on zoom level
    double gridSpacing = 1.0;
    while (gridSpacing * m_zoomFactor < 20.0) gridSpacing *= 10.0;
    while (gridSpacing * m_zoomFactor > 200.0) gridSpacing /= 10.0;
    
    // Calculate visible area
    float aspectRatio = static_cast<float>(width()) / height();
    float viewWidth = 10.0f / m_zoomFactor;
    float viewHeight = viewWidth / aspectRatio;
    
    double minX = -viewWidth/2 - m_panOffset.x() / m_zoomFactor / width() * viewWidth;
    double maxX = viewWidth/2 - m_panOffset.x() / m_zoomFactor / width() * viewWidth;
    double minY = -viewHeight/2 + m_panOffset.y() / m_zoomFactor / height() * viewHeight;
    double maxY = viewHeight/2 + m_panOffset.y() / m_zoomFactor / height() * viewHeight;
    
    // Draw vertical grid lines
    glBegin(GL_LINES);
    for (double x = floor(minX / gridSpacing) * gridSpacing; x <= maxX; x += gridSpacing) {
        glVertex2d(x, minY);
        glVertex2d(x, maxY);
    }
    
    // Draw horizontal grid lines
    for (double y = floor(minY / gridSpacing) * gridSpacing; y <= maxY; y += gridSpacing) {
        glVertex2d(minX, y);
        glVertex2d(maxX, y);
    }
    glEnd();
}

void GeometryViewer::drawAxes()
{
    glColor3f(m_axisColor.redF(), m_axisColor.greenF(), m_axisColor.blueF());
    glLineWidth(2.0f);
    
    // Calculate visible area
    float aspectRatio = static_cast<float>(width()) / height();
    float viewWidth = 10.0f / m_zoomFactor;
    float viewHeight = viewWidth / aspectRatio;
    
    double minX = -viewWidth/2 - m_panOffset.x() / m_zoomFactor / width() * viewWidth;
    double maxX = viewWidth/2 - m_panOffset.x() / m_zoomFactor / width() * viewWidth;
    double minY = -viewHeight/2 + m_panOffset.y() / m_zoomFactor / height() * viewHeight;
    double maxY = viewHeight/2 + m_panOffset.y() / m_zoomFactor / height() * viewHeight;
    
    // Draw X axis
    if (minY <= 0.0 && maxY >= 0.0) {
        glBegin(GL_LINES);
        glVertex2d(minX, 0.0);
        glVertex2d(maxX, 0.0);
        glEnd();
    }
    
    // Draw Y axis  
    if (minX <= 0.0 && maxX >= 0.0) {
        glBegin(GL_LINES);
        glVertex2d(0.0, minY);
        glVertex2d(0.0, maxY);
        glEnd();
    }
}

void GeometryViewer::mousePressEvent(QMouseEvent *event)
{
    m_mousePressed = true;
    m_lastMousePos = event->pos();
    m_mouseButton = event->button();
    setFocus();
}

void GeometryViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    // Convert mouse position to world coordinates for status display
    QPointF worldPos = screenToWorld(event->pos());
    emit mousePositionChanged(worldPos.x(), worldPos.y());
    
    if (m_mousePressed) {
        if (m_mouseButton == Qt::LeftButton) {
            // Pan
            m_panOffset.setX(m_panOffset.x() + delta.x() * PAN_SENSITIVITY);
            m_panOffset.setY(m_panOffset.y() + delta.y() * PAN_SENSITIVITY);
            update();
        } else if (m_mouseButton == Qt::RightButton) {
            // Rotate
            m_rotationAngle += delta.x() * ROTATION_SENSITIVITY;
            while (m_rotationAngle >= 360.0f) m_rotationAngle -= 360.0f;
            while (m_rotationAngle < 0.0f) m_rotationAngle += 360.0f;
            update();
        }
    }
}

void GeometryViewer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_mousePressed = false;
    m_mouseButton = Qt::NoButton;
}

void GeometryViewer::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f; // Standard wheel step
    float zoomChange = 1.0f + delta * ZOOM_SENSITIVITY;
    
    m_zoomFactor *= zoomChange;
    m_zoomFactor = qBound(MIN_ZOOM, m_zoomFactor, MAX_ZOOM);
    
    update();
}

void GeometryViewer::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_R:
            resetView();
            break;
        case Qt::Key_F:
            fitToWindow();
            break;
        case Qt::Key_G:
            m_showGrid = !m_showGrid;
            update();
            break;
        case Qt::Key_A:
            m_showAxes = !m_showAxes;
            update();
            break;
        default:
            QOpenGLWidget::keyPressEvent(event);
    }
}

void GeometryViewer::animate()
{
    // Update animation if needed
    // Currently no animations, but this can be used for future features
}

QPointF GeometryViewer::screenToWorld(const QPoint &screenPos) const
{
    // Convert screen coordinates to world coordinates
    float aspectRatio = static_cast<float>(width()) / height();
    float viewWidth = 10.0f / m_zoomFactor;
    float viewHeight = viewWidth / aspectRatio;
    
    double normalizedX = (2.0 * screenPos.x() / width()) - 1.0;
    double normalizedY = 1.0 - (2.0 * screenPos.y() / height());
    
    double worldX = normalizedX * viewWidth / 2.0 - m_panOffset.x() / m_zoomFactor / width() * viewWidth;
    double worldY = normalizedY * viewHeight / 2.0 + m_panOffset.y() / m_zoomFactor / height() * viewHeight;
    
    return QPointF(worldX, worldY);
}

QPoint GeometryViewer::worldToScreen(const QPointF &worldPos) const
{
    // Convert world coordinates to screen coordinates
    float aspectRatio = static_cast<float>(width()) / height();
    float viewWidth = 10.0f / m_zoomFactor;
    float viewHeight = viewWidth / aspectRatio;
    
    double normalizedX = (worldPos.x() + m_panOffset.x() / m_zoomFactor / width() * viewWidth) / (viewWidth / 2.0);
    double normalizedY = (worldPos.y() - m_panOffset.y() / m_zoomFactor / height() * viewHeight) / (viewHeight / 2.0);
    
    int screenX = static_cast<int>((normalizedX + 1.0) * width() / 2.0);
    int screenY = static_cast<int>((1.0 - normalizedY) * height() / 2.0);
    
    return QPoint(screenX, screenY);
}

void GeometryViewer::updateBounds()
{
    if (!m_geometryCollection) {
        m_boundsValid = false;
        return;
    }
    
    double minX, minY, maxX, maxY;
    if (geometry_collection_bounding_box(m_geometryCollection, &minX, &minY, &maxX, &maxY) == 0) {
        m_minX = minX;
        m_minY = minY;
        m_maxX = maxX;
        m_maxY = maxY;
        m_boundsValid = true;
    } else {
        m_boundsValid = false;
    }
}