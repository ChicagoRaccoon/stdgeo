

#pragma once

#include "CommonHeader.h"


#include <GL/glew.h>  // Must be included before other GL headers. Not sure if this is necessary in this file.
#include <wx/wx.h>
#include "GLCanvas.h"
#include "Terminal.h"

// TODO I don't know the best way or place to put these
// Event IDs for toolbar buttons and controls
enum
{
    ID_VIEW_FRONT = wxID_HIGHEST + 1,
    ID_VIEW_TOP,
    ID_VIEW_SIDE,
    ID_VIEW_ISOMETRIC,
    ID_VIEW_RESET,
    ID_TERMINAL
};

class Toolbar : public wxToolBar
{
public:
    Toolbar(wxWindow* parent, GLCanvas* pGlCanvas, Terminal* pTerminal);
    ~Toolbar();

    // Event handlers for toolbar buttons
    void  OnViewFront      (wxCommandEvent& event);
    void  OnViewTop        (wxCommandEvent& event);
    void  OnViewSide       (wxCommandEvent& event);
    void  OnViewIsometric  (wxCommandEvent& event);
    void  OnResetView      (wxCommandEvent& event);

private:
    // TODO These should be replaced / handled differently
    GLCanvas*     m_pGlCanvas;   // 3D rendering viewport
    Terminal*     m_pTerminal;   // Terminal-style text control
};

