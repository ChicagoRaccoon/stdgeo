// Main application window with dockable panels: OpenGL canvas, text output, and toolbar
#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include "GLCanvas.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

    void AppendOutput(const wxString& text);  // Add text to terminal

private:
    // Event handlers for toolbar buttons
    void OnViewFront(wxCommandEvent& event);
    void OnViewTop(wxCommandEvent& event);
    void OnViewSide(wxCommandEvent& event);
    void OnViewIsometric(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);

    // Event handlers for terminal input
    void OnTerminalChar(wxKeyEvent& event);

    // Terminal helpers
    void ExecuteCommand(const wxString& command);
    void ShowPrompt();
    wxString GetCurrentLine();
    void ClearCurrentLine();
    void RestrictEditableArea();

    // UI components
    wxAuiManager m_auiManager;  // Manages dockable panes
    GLCanvas* m_glCanvas;       // 3D rendering viewport
    wxTextCtrl* m_terminal;     // Terminal-style text control
    wxToolBar* m_toolBar;       // View control toolbar

    // Terminal state
    long m_promptPos;           // Start position of current input
    std::vector<wxString> m_history;  // Command history
    int m_historyIndex;         // Current position in history

    wxDECLARE_EVENT_TABLE();
};