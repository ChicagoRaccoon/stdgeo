// MainFrame implementation: creates split window with GL canvas and text pane
#include "MainFrame.h"
#include <wx/splitter.h>

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
{
    // Create splitter to divide window into two resizable panes
    wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY);

    // Configure OpenGL context attributes (RGBA, double buffer, 16-bit depth)
    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

    // Create OpenGL rendering canvas (left pane)
    m_glCanvas = new GLCanvas(splitter, canvasAttrs);

    // Create read-only multiline text control (right pane)
    m_textCtrl = new wxTextCtrl(splitter, wxID_ANY,
                                "Text output window\n\nThis is a multiline text box.\n",
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);

    // Split window vertically: 800px for GL canvas, remainder for text
    splitter->SplitVertically(m_glCanvas, m_textCtrl, 800);
    splitter->SetMinimumPaneSize(200);

    // Add status bar with usage instructions
    CreateStatusBar();
    SetStatusText("Ready - Left mouse: rotate | Right mouse: pan | Scroll: zoom");
}