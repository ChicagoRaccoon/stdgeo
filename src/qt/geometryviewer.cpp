#include "geometryviewer.h"
#include <QDebug>
#include <cmath>

static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;

void main()
{
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

GeometryViewer::GeometryViewer(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_program(nullptr)
    , m_cameraDistance(5.0f)
    , m_cameraRotationX(20.0f)
    , m_cameraRotationY(45.0f)
    , m_mousePressed(false)
    , m_currentMesh(nullptr)
    , m_meshNeedsUpdate(false)
{
    setFocusPolicy(Qt::StrongFocus);
}

GeometryViewer::~GeometryViewer()
{
    makeCurrent();
    if (m_currentMesh) {
        mesh_free(m_currentMesh);
    }
    delete m_program;
    doneCurrent();
}

void GeometryViewer::initializeGL()
{
    initializeOpenGLFunctions();
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    setupShaders();
    setupGeometry();
    setupCamera();
}

void GeometryViewer::setupShaders()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    
    if (!m_program->isLinked()) {
        qDebug() << "Shader program failed to link:" << m_program->log();
    }
}

void GeometryViewer::setupGeometry()
{
    m_vao.create();
    m_vao.bind();
    
    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertexBuffer.bind();
    
    m_indexBuffer.create();
    m_indexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    
    // Set up vertex attributes for position (0) and normal (1)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    m_vao.release();
}

void GeometryViewer::setupCamera()
{
    m_model.setToIdentity();
    // Set initial camera position to be able to see a default cube
    m_cameraDistance = 5.0f;
    m_cameraRotationX = 20.0f;
    m_cameraRotationY = 45.0f;
}

void GeometryViewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (m_meshNeedsUpdate) {
        updateMesh();
        m_meshNeedsUpdate = false;
    }
    
    if (m_vertices.isEmpty()) {
        return;
    }
    
    m_program->bind();
    
    // Update view matrix based on camera position
    m_view.setToIdentity();
    m_view.translate(0, 0, -m_cameraDistance);
    m_view.rotate(m_cameraRotationX, 1, 0, 0);
    m_view.rotate(m_cameraRotationY, 0, 1, 0);
    
    // Set uniforms
    m_program->setUniformValue("model", m_model);
    m_program->setUniformValue("view", m_view);
    m_program->setUniformValue("projection", m_projection);
    m_program->setUniformValue("lightPos", QVector3D(2.0f, 2.0f, 2.0f));
    m_program->setUniformValue("lightColor", QVector3D(1.0f, 1.0f, 1.0f));
    m_program->setUniformValue("objectColor", QVector3D(0.7f, 0.3f, 0.3f));
    m_program->setUniformValue("viewPos", QVector3D(0, 0, m_cameraDistance));
    
    m_vao.bind();
    m_indexBuffer.bind();
    
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    
    m_vao.release();
    m_program->release();
}

void GeometryViewer::resizeGL(int width, int height)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, float(width) / float(height), 0.1f, 100.0f);
}

void GeometryViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
    }
}

void GeometryViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
    }
}

void GeometryViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressed && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_cameraRotationY += delta.x() * 0.5f;
        m_cameraRotationX += delta.y() * 0.5f;
        
        // Clamp X rotation
        m_cameraRotationX = qBound(-90.0f, m_cameraRotationX, 90.0f);
        
        m_lastMousePos = event->pos();
        update();
    }
}

void GeometryViewer::wheelEvent(QWheelEvent *event)
{
    if (!event) return;
    
    QPoint angleDelta = event->angleDelta();
    if (!angleDelta.isNull()) {
        float delta = angleDelta.y() / 120.0f;
        m_cameraDistance -= delta * 0.5f;
        m_cameraDistance = qBound(1.0f, m_cameraDistance, 20.0f);
        update();
    }
    event->accept();
}

void GeometryViewer::clearMesh()
{
    if (m_currentMesh) {
        mesh_free(m_currentMesh);
        m_currentMesh = nullptr;
    }
    m_vertices.clear();
    m_indices.clear();
    m_meshNeedsUpdate = true;
    update();
}

void GeometryViewer::createCube(double size)
{
    clearMesh();
    m_currentMesh = mesh_create_cube(size);
    if (m_currentMesh) {
        qDebug() << "Created cube with" << mesh_triangle_count(m_currentMesh) << "triangles";
    } else {
        qDebug() << "Failed to create cube";
    }
    m_meshNeedsUpdate = true;
    update();
}

void GeometryViewer::setMesh(Mesh* mesh)
{
    clearMesh();
    m_currentMesh = mesh;
    m_meshNeedsUpdate = true;
    update();
}

void GeometryViewer::updateMesh()
{
    m_vertices.clear();
    m_indices.clear();
    
    if (!m_currentMesh) {
        // Update OpenGL buffers even when clearing mesh
        if (context()) {
            makeCurrent();
            m_vao.bind();
            m_vertexBuffer.bind();
            m_vertexBuffer.allocate(nullptr, 0);
            m_indexBuffer.bind();
            m_indexBuffer.allocate(nullptr, 0);
            m_vao.release();
            doneCurrent();
        }
        return;
    }
    
    size_t triangleCount = mesh_triangle_count(m_currentMesh);
    qDebug() << "Updating mesh with" << triangleCount << "triangles";
    
    for (size_t i = 0; i < triangleCount; ++i) {
        double v0_x, v0_y, v0_z, v1_x, v1_y, v1_z, v2_x, v2_y, v2_z;
        
        if (mesh_get_triangle_vertices(m_currentMesh, i,
                                     &v0_x, &v0_y, &v0_z,
                                     &v1_x, &v1_y, &v1_z,
                                     &v2_x, &v2_y, &v2_z)) {
            
            // Calculate normal for the triangle
            QVector3D edge1(v1_x - v0_x, v1_y - v0_y, v1_z - v0_z);
            QVector3D edge2(v2_x - v0_x, v2_y - v0_y, v2_z - v0_z);
            QVector3D normal = QVector3D::crossProduct(edge1, edge2).normalized();
            
            // Add vertices with normals
            unsigned int baseIndex = m_vertices.size() / 6;
            
            // Vertex 0
            m_vertices << v0_x << v0_y << v0_z << normal.x() << normal.y() << normal.z();
            // Vertex 1
            m_vertices << v1_x << v1_y << v1_z << normal.x() << normal.y() << normal.z();
            // Vertex 2
            m_vertices << v2_x << v2_y << v2_z << normal.x() << normal.y() << normal.z();
            
            // Add indices
            m_indices << baseIndex << baseIndex + 1 << baseIndex + 2;
        }
    }
    
    // Update OpenGL buffers only if context is available
    if (context() && m_vao.isCreated()) {
        makeCurrent();
        
        m_vao.bind();
        
        m_vertexBuffer.bind();
        if (!m_vertices.isEmpty()) {
            m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.size() * sizeof(float));
        }
        
        m_indexBuffer.bind();
        if (!m_indices.isEmpty()) {
            m_indexBuffer.allocate(m_indices.constData(), m_indices.size() * sizeof(unsigned int));
        }
        
        m_vao.release();
        
        doneCurrent();
    }
    
    qDebug() << "Mesh updated with" << (m_vertices.size() / 6) << "vertices and" << (m_indices.size() / 3) << "triangles";
}