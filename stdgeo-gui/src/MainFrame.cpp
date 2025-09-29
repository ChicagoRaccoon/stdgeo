// MainFrame implementation: creates dockable window with GL canvas, text pane, and toolbar
#include "MainFrame.h"
#include <wx/artprov.h>

// Event IDs for toolbar buttons
enum {
    ID_VIEW_FRONT = wxID_HIGHEST + 1,
    ID_VIEW_TOP,
    ID_VIEW_SIDE,
    ID_VIEW_ISOMETRIC,
    ID_VIEW_RESET
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TOOL(ID_VIEW_FRONT, MainFrame::OnViewFront)
    EVT_TOOL(ID_VIEW_TOP, MainFrame::OnViewTop)
    EVT_TOOL(ID_VIEW_SIDE, MainFrame::OnViewSide)
    EVT_TOOL(ID_VIEW_ISOMETRIC, MainFrame::OnViewIsometric)
    EVT_TOOL(ID_VIEW_RESET, MainFrame::OnResetView)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
{
    // Initialize AUI manager for dockable panes
    m_auiManager.SetManagedWindow(this);

    // Create toolbar with view preset buttons
    m_toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxTB_FLAT | wxTB_NODIVIDER);
    m_toolBar->AddTool(ID_VIEW_FRONT, "Front", wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR),
                       "View from front");
    m_toolBar->AddTool(ID_VIEW_TOP, "Top", wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR),
                       "View from top");
    m_toolBar->AddTool(ID_VIEW_SIDE, "Side", wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR),
                       "View from side");
    m_toolBar->AddTool(ID_VIEW_ISOMETRIC, "Iso", wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR),
                       "Isometric view");
    m_toolBar->AddSeparator();
    m_toolBar->AddTool(ID_VIEW_RESET, "Reset", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR),
                       "Reset view");
    m_toolBar->Realize();

    // Configure OpenGL context attributes (RGBA, double buffer, 16-bit depth)
    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

    // Create OpenGL rendering canvas
    m_glCanvas = new GLCanvas(this, canvasAttrs);

    // Create multiline text control
    m_textCtrl = new wxTextCtrl(this, wxID_ANY, "=== StdGeo Output Log ===\n\n",
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);

    // Add toolbar as a dockable pane (top, floatable)
    m_auiManager.AddPane(m_toolBar, wxAuiPaneInfo()
                        .Name("toolbar")
                        .Caption("View Controls")
                        .ToolbarPane()
                        .Top()
                        .Floatable()
                        .Gripper());

    // Add GL canvas as central pane (fills remaining space, floatable)
    m_auiManager.AddPane(m_glCanvas, wxAuiPaneInfo()
                        .Name("canvas")
                        .Caption("3D Viewport")
                        .Center()
                        .CloseButton(false)
                        .Floatable()
                        .MaximizeButton());

    // Add text control as dockable pane (right side, floatable)
    m_auiManager.AddPane(m_textCtrl, wxAuiPaneInfo()
                        .Name("output")
                        .Caption("Output Log")
                        .Right()
                        .BestSize(350, -1)
                        .MinSize(200, -1)
                        .Floatable()
                        .CloseButton()
                        .MaximizeButton());

    // Commit all pane configurations
    m_auiManager.Update();

    // Add status bar with usage instructions
    CreateStatusBar();
    SetStatusText("Ready - Left mouse: rotate | Right mouse: pan | Scroll: zoom | Drag panes to dock/undock");
}

MainFrame::~MainFrame() {
    // Clean up AUI manager
    m_auiManager.UnInit();
}

void MainFrame::AppendText(const wxString& text) {
    m_textCtrl->AppendText(text + "\n");
}

// Toolbar button handlers
void MainFrame::OnViewFront(wxCommandEvent& event) {
    m_glCanvas->SetViewFront();
    AppendText("View: Front (rotation reset, facing +Z)");
}

void MainFrame::OnViewTop(wxCommandEvent& event) {
    m_glCanvas->SetViewTop();
    AppendText("View: Top (looking down -Y axis)");
}

void MainFrame::OnViewSide(wxCommandEvent& event) {
    m_glCanvas->SetViewSide();
    AppendText("View: Side (looking from +X axis)");
}

void MainFrame::OnViewIsometric(wxCommandEvent& event) {
    m_glCanvas->SetViewIsometric();
    AppendText("View: Isometric (45° rotation on X and Y)");
}

void MainFrame::OnResetView(wxCommandEvent& event) {
    m_glCanvas->ResetView();
    AppendText("View: Reset to default position");
}