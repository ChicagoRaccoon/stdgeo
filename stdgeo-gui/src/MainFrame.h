// Main application window with split view: OpenGL canvas and text output
#pragma once

#include <wx/wx.h>
#include "GLCanvas.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);

private:
    GLCanvas* m_glCanvas;      // 3D rendering viewport
    wxTextCtrl* m_textCtrl;    // Text output/log window
};