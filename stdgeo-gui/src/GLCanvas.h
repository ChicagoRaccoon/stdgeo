// OpenGL rendering canvas with interactive 3D cube viewer
// Provides pan, zoom, and rotate controls via mouse input
#pragma once

#include "CommonHeader.h"

#include <GL/glew.h>  // Must be included before other GL headers
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>

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
    void OnLeftClick(wxMouseEvent& event);

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
    void CreateEdgeBuffers();
    GLuint LoadShaders(const char* vertexPath, const char* fragmentPath);

    // Face picking
    int PickFace(int mouseX, int mouseY);
    glm::vec3 ScreenToWorldRay(int mouseX, int mouseY);
    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                               const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                               float& t);

    wxGLContext* m_context;
    bool m_glInitialized;

    // OpenGL objects
    GLuint m_shaderProgram;  // Compiled shader program
    GLuint m_VAO, m_VBO;     // Vertex array and buffer objects
    GLuint m_edgeVAO, m_edgeVBO;  // Edge rendering objects
    GLuint m_mvpLocation;    // Model-View-Projection uniform location
    GLuint m_colorLocation;  // Color uniform location for edge rendering

    // Camera/view parameters
    float m_zoom;            // Distance from camera to object
    glm::vec3 m_rotation;    // Rotation angles (x, y, z)
    glm::vec2 m_pan;         // Pan offset (x, y)

    // Mouse interaction state
    wxPoint m_lastMousePos;
    wxPoint m_mouseDownPos;  // Track where mouse button was pressed
    bool m_isRotating;       // Left mouse button active
    bool m_isPanning;        // Right mouse button active

    // Face selection state
    std::set<int> m_selectedFaces;  // Set of selected face indices (0-5)

    wxDECLARE_EVENT_TABLE();
};