#pragma once

#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GLCanvas : public wxGLCanvas {
public:
    GLCanvas(wxWindow* parent, const wxGLAttributes& canvasAttrs);
    ~GLCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);

private:
    void InitGL();
    void Render();
    void CreateShaderProgram();
    void CreateCube();
    GLuint LoadShaders(const char* vertexPath, const char* fragmentPath);

    wxGLContext* m_context;
    bool m_glInitialized;

    // OpenGL objects
    GLuint m_shaderProgram;
    GLuint m_VAO, m_VBO;
    GLuint m_mvpLocation;

    // Camera/view parameters
    float m_zoom;
    glm::vec3 m_rotation;
    glm::vec2 m_pan;

    // Mouse interaction
    wxPoint m_lastMousePos;
    bool m_isRotating;
    bool m_isPanning;

    wxDECLARE_EVENT_TABLE();
};