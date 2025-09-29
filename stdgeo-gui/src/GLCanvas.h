// OpenGL rendering canvas with interactive 3D cube viewer
// Provides pan, zoom, and rotate controls via mouse input
#pragma once

#include <GL/glew.h>  // Must be included before other GL headers
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GLCanvas : public wxGLCanvas {
public:
    GLCanvas(wxWindow* parent, const wxGLAttributes& canvasAttrs);
    ~GLCanvas();

    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);

    // View presets
    void SetViewFront();
    void SetViewTop();
    void SetViewSide();
    void SetViewIsometric();
    void ResetView();

private:
    // OpenGL initialization and rendering
    void InitGL();
    void Render();
    void CreateShaderProgram();
    void CreateCube();
    GLuint LoadShaders(const char* vertexPath, const char* fragmentPath);

    wxGLContext* m_context;
    bool m_glInitialized;

    // OpenGL objects
    GLuint m_shaderProgram;  // Compiled shader program
    GLuint m_VAO, m_VBO;     // Vertex array and buffer objects
    GLuint m_mvpLocation;    // Model-View-Projection uniform location

    // Camera/view parameters
    float m_zoom;            // Distance from camera to object
    glm::vec3 m_rotation;    // Rotation angles (x, y, z)
    glm::vec2 m_pan;         // Pan offset (x, y)

    // Mouse interaction state
    wxPoint m_lastMousePos;
    bool m_isRotating;       // Left mouse button active
    bool m_isPanning;        // Right mouse button active

    wxDECLARE_EVENT_TABLE();
};