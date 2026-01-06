// Main application window with dockable panels: OpenGL canvas, text output, and toolbar
#pragma once

#include "CommonHeader.h"

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "GLCanvas.h"
#include "Terminal.h"
#include "Toolbar.h"

class MainFrame : public wxFrame
{
public:
    MainFrame   (const wxString& title);
    ~MainFrame  (void);


private:

    // UI components
    wxAuiManager  m_auiManager;  // Manages dockable panes
    GLCanvas*     m_pGlCanvas;   // 3D rendering viewport
    Terminal*     m_pTerminal;   // Terminal-style text control
    Toolbar*      m_pToolBar;    // View control toolbar



    wxDECLARE_EVENT_TABLE();
};

