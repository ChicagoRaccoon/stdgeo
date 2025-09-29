// Implementation of OpenGL canvas with interactive 3D cube
#include "GLCanvas.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <wx/stdpaths.h>
#include <wx/filename.h>

// Get full path to shader file relative to executable directory
static wxString GetShaderPath(const wxString& shaderFile) {
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
    EVT_PAINT(GLCanvas::OnPaint)
    EVT_SIZE(GLCanvas::OnSize)
    EVT_MOUSEWHEEL(GLCanvas::OnMouseWheel)
    EVT_MOTION(GLCanvas::OnMouseMove)
    EVT_LEFT_DOWN(GLCanvas::OnMouseDown)
    EVT_LEFT_UP(GLCanvas::OnMouseUp)
    EVT_RIGHT_DOWN(GLCanvas::OnMouseDown)
    EVT_RIGHT_UP(GLCanvas::OnMouseUp)
wxEND_EVENT_TABLE()

GLCanvas::GLCanvas(wxWindow* parent, const wxGLAttributes& canvasAttrs)
    : wxGLCanvas(parent, canvasAttrs, wxID_ANY, wxDefaultPosition, wxDefaultSize),
      m_context(nullptr),
      m_glInitialized(false),
      m_shaderProgram(0),
      m_VAO(0),
      m_VBO(0),
      m_zoom(5.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_pan(0.0f, 0.0f),
      m_isRotating(false),
      m_isPanning(false)
{
    m_context = new wxGLContext(this);
}

GLCanvas::~GLCanvas() {
    if (m_context) {
        SetCurrent(*m_context);
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
        delete m_context;
    }
}

// Initialize OpenGL context, load extensions, and create rendering resources
void GLCanvas::InitGL() {
    SetCurrent(*m_context);

    // Initialize GLEW to access modern OpenGL functions
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
        return;
    }

    // Enable depth testing and set background color
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    CreateShaderProgram();
    CreateCube();

    m_glInitialized = true;
}

// Load, compile, and link vertex and fragment shaders into a program
GLuint GLCanvas::LoadShaders(const char* vertexPath, const char* fragmentPath) {
    // Locate shader files
    wxString vertexFullPath = GetShaderPath(wxString(vertexPath));
    wxString fragmentFullPath = GetShaderPath(wxString(fragmentPath));

    if (vertexFullPath.IsEmpty() || fragmentFullPath.IsEmpty()) {
        std::cerr << "Failed to find shader files" << std::endl;
        return 0;
    }

    // Read shader source code
    std::ifstream vShaderFile(vertexFullPath.ToStdString());
    std::ifstream fShaderFile(fragmentFullPath.ToStdString());

    if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
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
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
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
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    // Clean up individual shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void GLCanvas::CreateShaderProgram() {
    m_shaderProgram = LoadShaders("vertex.glsl", "fragment.glsl");
    if (m_shaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }
    m_mvpLocation = glGetUniformLocation(m_shaderProgram, "MVP");
}

// Create colored cube geometry (6 faces, 2 triangles each, 36 vertices total)
void GLCanvas::CreateCube() {
    // Vertex data: position (x,y,z) + color (r,g,b)
    float vertices[] = {
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

// Render the scene with current camera parameters
void GLCanvas::Render() {
    if (!m_glInitialized) return;

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

    // Draw the cube
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    SwapBuffers();
}

void GLCanvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);

    if (!m_glInitialized) {
        InitGL();
    }

    Render();
}

void GLCanvas::OnSize(wxSizeEvent& event) {
    if (m_glInitialized) {
        SetCurrent(*m_context);
        wxSize size = GetSize();
        glViewport(0, 0, size.x, size.y);
    }
    event.Skip();
}

// Mouse wheel: zoom in/out (clamped between 1 and 20)
void GLCanvas::OnMouseWheel(wxMouseEvent& event) {
    float delta = event.GetWheelRotation() / 120.0f;
    m_zoom -= delta * 0.5f;
    m_zoom = std::max(1.0f, std::min(20.0f, m_zoom));
    Refresh();
}

// Mouse move: rotate (left button) or pan (right button)
void GLCanvas::OnMouseMove(wxMouseEvent& event) {
    wxPoint currentPos = event.GetPosition();

    if (m_isRotating) {
        wxPoint delta = currentPos - m_lastMousePos;
        m_rotation.y += delta.x * 0.01f;
        m_rotation.x += delta.y * 0.01f;
        Refresh();
    }
    else if (m_isPanning) {
        wxPoint delta = currentPos - m_lastMousePos;
        m_pan.x += delta.x * 0.01f;
        m_pan.y -= delta.y * 0.01f;
        Refresh();
    }

    m_lastMousePos = currentPos;
}

// Mouse button down: begin rotate (left) or pan (right)
void GLCanvas::OnMouseDown(wxMouseEvent& event) {
    m_lastMousePos = event.GetPosition();

    if (event.LeftDown()) {
        m_isRotating = true;
    }
    else if (event.RightDown()) {
        m_isPanning = true;
    }
}

// Mouse button up: end rotate or pan
void GLCanvas::OnMouseUp(wxMouseEvent& event) {
    if (event.LeftUp()) {
        m_isRotating = false;
    }
    else if (event.RightUp()) {
        m_isPanning = false;
    }
}