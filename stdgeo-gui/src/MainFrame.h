// Main application window with dockable panels: OpenGL canvas, text output, and toolbar
#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "GLCanvas.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

    void AppendText(const wxString& text);  // Add text to output window

private:
    // Event handlers for toolbar buttons
    void OnViewFront(wxCommandEvent& event);
    void OnViewTop(wxCommandEvent& event);
    void OnViewSide(wxCommandEvent& event);
    void OnViewIsometric(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);

    // UI components
    wxAuiManager m_auiManager;  // Manages dockable panes
    GLCanvas* m_glCanvas;       // 3D rendering viewport
    wxTextCtrl* m_textCtrl;     // Text output/log window
    wxToolBar* m_toolBar;       // View control toolbar

    wxDECLARE_EVENT_TABLE();
};