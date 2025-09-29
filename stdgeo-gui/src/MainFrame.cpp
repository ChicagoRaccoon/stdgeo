#include "MainFrame.h"
#include <wx/splitter.h>

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
{
    // Create splitter window
    wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY);

    // Create OpenGL canvas attributes
    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

    // Create GL canvas
    m_glCanvas = new GLCanvas(splitter, canvasAttrs);

    // Create multiline text control
    m_textCtrl = new wxTextCtrl(splitter, wxID_ANY,
                                "Text output window\n\nThis is a multiline text box.\n",
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);

    // Split window vertically (left: GL canvas, right: text)
    splitter->SplitVertically(m_glCanvas, m_textCtrl, 800);
    splitter->SetMinimumPaneSize(200);

    // Create status bar
    CreateStatusBar();
    SetStatusText("Ready - Left mouse: rotate | Right mouse: pan | Scroll: zoom");
}