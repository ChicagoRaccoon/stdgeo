
/**
 * THEORY OF OPERATION
 * 
 * This module provides the implementation of an OpenGL canvas with interactive 3D cube.
 */

#include "CommonHeader.h"

#include "GLCanvas.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <wx/stdpaths.h>
#include <wx/filename.h>

// Get full path to shader file relative to executable directory
static wxString GetShaderPath(const wxString& shaderFile)
{
    wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
    wxString shaderPath = exePath.GetPath() + "/shaders/" + shaderFile;

    if (!wxFileExists(shaderPath)) {
        std::cerr << "Shader not found: " << shaderPath.ToStdString() << std::endl;
        return "";
    }

    return shaderPath;
}

// Event table: maps wxWidgets events to handler methods
wxBEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
    EVT_PAINT     ( GLCanvas::OnPaint      )
    EVT_SIZE      ( GLCanvas::OnSize       )
    EVT_MOUSEWHEEL( GLCanvas::OnMouseWheel )
    EVT_MOTION    ( GLCanvas::OnMouseMove  )
    EVT_LEFT_DOWN ( GLCanvas::OnMouseDown  )
    EVT_LEFT_UP   ( GLCanvas::OnLeftClick  )
    EVT_RIGHT_DOWN( GLCanvas::OnMouseDown  )
    EVT_RIGHT_UP  ( GLCanvas::OnMouseUp    )
wxEND_EVENT_TABLE()

GLCanvas::GLCanvas(wxWindow* parent, const wxGLAttributes& canvasAttrs)
    : wxGLCanvas(parent, canvasAttrs, wxID_ANY, wxDefaultPosition, wxDefaultSize),
      m_context(nullptr),
      m_glInitialized(false),
      m_shaderProgram(0),
      m_VAO(0),
      m_VBO(0),
      m_edgeVAO(0),
      m_edgeVBO(0),
      m_zoom(5.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_pan(0.0f, 0.0f),
      m_isRotating(false),
      m_isPanning(false)
{
    m_context = new wxGLContext(this);
}

GLCanvas::~GLCanvas() 
{
    if (m_context) 
    {
        SetCurrent(*m_context);

        if (m_VAO          )  glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO          )  glDeleteBuffers     (1, &m_VBO);
        if (m_edgeVAO      )  glDeleteVertexArrays(1, &m_edgeVAO);
        if (m_edgeVBO      )  glDeleteBuffers     (1, &m_edgeVBO);
        if (m_shaderProgram)  glDeleteProgram     (m_shaderProgram);

        delete m_context;
    }
}

// Initialize OpenGL context, load extensions, and create rendering resources
void GLCanvas::InitGL()
{
    SetCurrent(*m_context);

    // Initialize GLEW to access modern OpenGL functions
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
        return;
    }

    // Enable depth testing and set background color
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    CreateShaderProgram();
    CreateCube();
    CreateEdgeBuffers();

    m_glInitialized = true;
}

// Load, compile, and link vertex and fragment shaders into a program
GLuint GLCanvas::LoadShaders(const char* vertexPath, const char* fragmentPath)
{
    // Locate shader files
    wxString vertexFullPath = GetShaderPath(wxString(vertexPath));
    wxString fragmentFullPath = GetShaderPath(wxString(fragmentPath));

    if (vertexFullPath.IsEmpty() || fragmentFullPath.IsEmpty())
    {
        std::cerr << "Failed to find shader files" << std::endl;
        return 0;
    }

    // Read shader source code
    std::ifstream vShaderFile(vertexFullPath.ToStdString());
    std::ifstream fShaderFile(fragmentFullPath.ToStdString());

    if (!vShaderFile.is_open() || !fShaderFile.is_open())
    {
        std::cerr << "Failed to open shader files" << std::endl;
        return 0;
    }

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // Link shaders into program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    // Clean up individual shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void GLCanvas::CreateShaderProgram()
{
    m_shaderProgram = LoadShaders("vertex.glsl", "fragment.glsl");
    if (m_shaderProgram == 0)
    {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }
    m_mvpLocation = glGetUniformLocation(m_shaderProgram, "MVP");
    m_colorLocation = glGetUniformLocation(m_shaderProgram, "overrideColor");
}

// Create colored cube geometry (6 faces, 2 triangles each, 36 vertices total)
void GLCanvas::CreateCube()
{
    // Vertex data: position (x,y,z) + color (r,g,b)
    float vertices[] =
    {
        // Front face (red)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        // Back face (green)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

        // Left face (blue)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,

        // Right face (yellow)
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        // Top face (cyan)
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

        // Bottom face (magenta)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
    };

    // Create and bind OpenGL buffers
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configure vertex attributes: position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Configure vertex attributes: color (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Create edge geometry for highlighting selected faces
void GLCanvas::CreateEdgeBuffers()
{
    // Edge vertices for each face (4 edges per face, 2 vertices per edge)
    // Each face has edges defined as lines
    float edgeVertices[] =
    {
        // Face 0: Front (z = 0.5)
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  // Bottom edge
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  // Right edge
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  // Top edge
        -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  // Left edge

        // Face 1: Back (z = -0.5)
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  // Bottom edge
         0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  // Right edge
         0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  // Top edge
        -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  // Left edge

        // Face 2: Left (x = -0.5)
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  // Bottom edge
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  // Front edge
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,  // Top edge
        -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  // Back edge

        // Face 3: Right (x = 0.5)
         0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  // Bottom edge
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  // Front edge
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  // Top edge
         0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  // Back edge

        // Face 4: Top (y = 0.5)
        -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  // Left edge
        -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  // Front edge
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  // Right edge
         0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  // Back edge

        // Face 5: Bottom (y = -0.5)
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  // Left edge
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  // Front edge
         0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  // Right edge
         0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  // Back edge
    };

    glGenVertexArrays(1, &m_edgeVAO);
    glGenBuffers(1, &m_edgeVBO);

    glBindVertexArray(m_edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(edgeVertices), edgeVertices, GL_STATIC_DRAW);

    // Position attribute only (no color)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// Render the scene with current camera parameters
void GLCanvas::Render()
{
    if (!m_glInitialized)
    {
        return;
    }

    SetCurrent(*m_context);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_shaderProgram);

    // Build transformation matrices
    wxSize size = GetSize();
    float aspect = (float)size.x / (float)size.y;

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(m_pan.x, m_pan.y, -m_zoom));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Send combined MVP matrix to shader
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, &mvp[0][0]);

    // Disable color override for normal cube rendering
    glUniform4f(m_colorLocation, 0.0f, 0.0f, 0.0f, 0.0f);

    // Draw the cube
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Draw highlighted edges for selected faces
    if (!m_selectedFaces.empty())
    {
        glLineWidth(3.0f);

        // Disable vertex color attribute since edges don't have color data
        glDisableVertexAttribArray(1);

        glBindVertexArray(m_edgeVAO);

        // Use a bright yellow color for highlighting
        glUniform4f(m_colorLocation, 1.0f, 1.0f, 0.0f, 1.0f);

        for (int faceIdx : m_selectedFaces)
        {
            // Each face has 4 edges, 2 vertices per edge = 8 vertices
            glDrawArrays(GL_LINES, faceIdx * 8, 8);
        }

        glBindVertexArray(0);

        // Re-enable vertex color attribute for normal rendering
        glEnableVertexAttribArray(1);
        glLineWidth(1.0f);
    }

    SwapBuffers();
}

void GLCanvas::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    if (!m_glInitialized)
    {
        InitGL();
    }

    Render();
}

void GLCanvas::OnSize(wxSizeEvent& event)
{
    if (m_glInitialized)
    {
        SetCurrent(*m_context);
        wxSize size = GetSize();
        glViewport(0, 0, size.x, size.y);
    }
    event.Skip();
}

// Mouse wheel: zoom in/out (clamped between 1 and 20)
void GLCanvas::OnMouseWheel(wxMouseEvent& event)
{
    float delta = event.GetWheelRotation() / 120.0f;
    m_zoom -= delta * 0.5f;
    m_zoom = std::max(1.0f, std::min(20.0f, m_zoom));
    Refresh();
}

// Mouse move: rotate (left button) or pan (right button)
void GLCanvas::OnMouseMove(wxMouseEvent& event)
{
    wxPoint currentPos = event.GetPosition();

    if (m_isRotating)
    {
        wxPoint delta = currentPos - m_lastMousePos;
        m_rotation.y += delta.x * 0.01f;
        m_rotation.x += delta.y * 0.01f;
        Refresh();
    }
    else if (m_isPanning)
    {
        wxPoint delta = currentPos - m_lastMousePos;
        m_pan.x += delta.x * 0.01f;
        m_pan.y -= delta.y * 0.01f;
        Refresh();
    }

    m_lastMousePos = currentPos;
}

// Mouse button down: begin rotate (left) or pan (right)
void GLCanvas::OnMouseDown(wxMouseEvent& event)
{
    m_lastMousePos = event.GetPosition();
    m_mouseDownPos = event.GetPosition();

    if (event.LeftDown())
    {
        m_isRotating = true;
    }
    else if (event.RightDown())
    {
        m_isPanning = true;
    }
}

// Left mouse button up: handle face selection (if not dragging)
void GLCanvas::OnLeftClick(wxMouseEvent& event)
{
    m_isRotating = false;

    // Only process as click if mouse hasn't moved much (not a drag)
    wxPoint currentPos = event.GetPosition();
    int dx = abs(currentPos.x - m_mouseDownPos.x);
    int dy = abs(currentPos.y - m_mouseDownPos.y);

    // Threshold for click vs drag
    // TODO These should be pound-defines
    if (dx < 5 && dy < 5)
    {  
        int faceIdx = PickFace(currentPos.x, currentPos.y);

        std::cout << "Clicked at (" << currentPos.x << ", " << currentPos.y << "), face: " << faceIdx << std::endl;

        if (faceIdx >= 0)
        {
            if (event.ControlDown())
            {
                // CTRL held: toggle face in selection
                if (m_selectedFaces.count(faceIdx))
                {
                    m_selectedFaces.erase(faceIdx);
                    std::cout << "Deselected face " << faceIdx << std::endl;
                }
                else
                {
                    m_selectedFaces.insert(faceIdx);
                    std::cout << "Added face " << faceIdx << " to selection" << std::endl;
                }
            }
            else
            {
                // No CTRL: select only this face
                m_selectedFaces.clear();
                m_selectedFaces.insert(faceIdx);
                std::cout << "Selected face " << faceIdx << std::endl;
            }
            std::cout << "Total selected faces: " << m_selectedFaces.size() << std::endl;
            Refresh();
        }
    }
}

// Mouse button up: end pan
void GLCanvas::OnMouseUp(wxMouseEvent& event)
{
    if (event.RightUp())
    {
        m_isPanning = false;
    }
}

// View preset methods
void GLCanvas::SetViewFront()
{
    m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    m_pan = glm::vec2(0.0f, 0.0f);
    m_zoom = 5.0f;
    Refresh();
}

void GLCanvas::SetViewTop()
{
    m_rotation = glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f);  // -90° on X axis
    m_pan = glm::vec2(0.0f, 0.0f);
    m_zoom = 5.0f;
    Refresh();
}

void GLCanvas::SetViewSide()
{
    m_rotation = glm::vec3(0.0f, glm::half_pi<float>(), 0.0f);  // 90° on Y axis
    m_pan = glm::vec2(0.0f, 0.0f);
    m_zoom = 5.0f;
    Refresh();
}

void GLCanvas::SetViewIsometric()
{
    m_rotation = glm::vec3(glm::radians(35.26f), glm::radians(45.0f), 0.0f);  // Standard isometric angles
    m_pan = glm::vec2(0.0f, 0.0f);
    m_zoom = 5.0f;
    Refresh();
}

void GLCanvas::ResetView()
{
    m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    m_pan = glm::vec2(0.0f, 0.0f);
    m_zoom = 5.0f;
    Refresh();
}

// Ray-triangle intersection using Möller-Trumbore algorithm
bool GLCanvas::RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                     const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                     float& t)
{
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
    {
        return false;  // Ray is parallel to triangle
    }

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
    {
        return false;
    }

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);

    if (v < 0.0f || u + v > 1.0f)
    {
        return false;
    }

    t = f * glm::dot(edge2, q);
    return t > EPSILON;
}

// Convert screen coordinates to world-space ray
glm::vec3 GLCanvas::ScreenToWorldRay(int mouseX, int mouseY)
{
    wxSize size = GetSize();
    float aspect = (float)size.x / (float)size.y;

    // Convert to normalized device coordinates
    float x = (2.0f * mouseX) / size.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / size.y;

    // Build inverse of view-projection matrix (NOT including model matrix)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(m_pan.x, m_pan.y, -m_zoom));

    glm::mat4 invVP = glm::inverse(projection * view);

    // Transform near and far points from clip space to world space
    glm::vec4 rayStart = invVP * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 rayEnd = invVP * glm::vec4(x, y, 1.0f, 1.0f);

    rayStart /= rayStart.w;  // Perspective divide
    rayEnd /= rayEnd.w;

    return glm::normalize(glm::vec3(rayEnd - rayStart));
}

// Pick a face based on mouse coordinates using ray casting
int GLCanvas::PickFace(int mouseX, int mouseY) {
    // Get camera position in view space (before rotation)
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(m_pan.x, m_pan.y, -m_zoom));
    glm::vec3 cameraPos = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));

    glm::vec3 rayDir = ScreenToWorldRay(mouseX, mouseY);

    std::cout << "Camera pos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "Ray dir: (" << rayDir.x << ", " << rayDir.y << ", " << rayDir.z << ")" << std::endl;

    // Define cube face vertices (matching the order in CreateCube)
    glm::vec3 faceVertices[6][6] =
    {
        // Face 0: Front (red)
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f),
          glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f) },
        // Face 1: Back (green)
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f),
          glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f) },
        // Face 2: Left (blue)
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f),
          glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f) },
        // Face 3: Right (yellow)
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f),
          glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f) },
        // Face 4: Top (cyan)
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f),
          glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f, -0.5f) },
        // Face 5: Bottom (magenta)
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f),
          glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f) }
    };

    // Apply model transformation to vertices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    // Find closest intersected face
    float closestDist = std::numeric_limits<float>::max();
    int closestFace = -1;

    for (int face = 0; face < 6; face++)
    {
        for (int tri = 0; tri < 2; tri++)
        {
            int idx = tri * 3;
            glm::vec3 v0 = glm::vec3(model * glm::vec4(faceVertices[face][idx + 0], 1.0f));
            glm::vec3 v1 = glm::vec3(model * glm::vec4(faceVertices[face][idx + 1], 1.0f));
            glm::vec3 v2 = glm::vec3(model * glm::vec4(faceVertices[face][idx + 2], 1.0f));

            float t;
            if (RayIntersectsTriangle(cameraPos, rayDir, v0, v1, v2, t))
            {
                if (t < closestDist)
                {
                    closestDist = t;
                    closestFace = face;
                }
            }
        }
    }

    return closestFace;
}


