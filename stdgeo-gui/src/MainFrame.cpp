
/**
 * THEORY OF OPERATION
 * 
 * MainFrame implementation.
 * Creates a dockable window with:
 *   - GL Canvas
 *   - Text Pane
 *   - Toolbar
 */

#include "CommonHeader.h"

#include "MainFrame.h"
#include <wx/artprov.h>
#include <vector>

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

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TOOL(ID_VIEW_FRONT    , MainFrame::OnViewFront    )
    EVT_TOOL(ID_VIEW_TOP      , MainFrame::OnViewTop      )
    EVT_TOOL(ID_VIEW_SIDE     , MainFrame::OnViewSide     )
    EVT_TOOL(ID_VIEW_ISOMETRIC, MainFrame::OnViewIsometric)
    EVT_TOOL(ID_VIEW_RESET    , MainFrame::OnResetView    )
wxEND_EVENT_TABLE()



MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
{
    // Initialize AUI manager for dockable panes
    m_auiManager.SetManagedWindow(this);

    // Create toolbar with view preset buttons
    m_pToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);

    m_pToolBar->AddTool(ID_VIEW_FRONT    , "Front", wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR), "View from front");
    m_pToolBar->AddTool(ID_VIEW_TOP      , "Top"  , wxArtProvider::GetBitmap(wxART_GO_UP     , wxART_TOOLBAR), "View from top"  );
    m_pToolBar->AddTool(ID_VIEW_SIDE     , "Side" , wxArtProvider::GetBitmap(wxART_GO_BACK   , wxART_TOOLBAR), "View from side" );
    m_pToolBar->AddTool(ID_VIEW_ISOMETRIC, "Iso"  , wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR), "Isometric view" );
    m_pToolBar->AddSeparator();
    m_pToolBar->AddTool(ID_VIEW_RESET    , "Reset", wxArtProvider::GetBitmap(wxART_UNDO      , wxART_TOOLBAR), "Reset view"     );
    m_pToolBar->Realize();

    // Configure OpenGL context attributes (RGBA, double buffer, 16-bit depth)
    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

    // Create OpenGL rendering canvas
    m_pGlCanvas = new GLCanvas(this, canvasAttrs);

    // Create terminal-style text control
    m_pTerminal = new Terminal(this, ID_TERMINAL, m_pGlCanvas);

    // Add toolbar as a dockable pane (top, floatable)
    m_auiManager.AddPane(m_pToolBar, wxAuiPaneInfo()
                        .Name("toolbar")
                        .Caption("View Controls")
                        .ToolbarPane()
                        .Top()
                        .Floatable()
                        .Gripper());

    // Add GL canvas as central pane (fills remaining space, floatable)
    m_auiManager.AddPane(m_pGlCanvas, wxAuiPaneInfo()
                        .Name("canvas")
                        .Caption("3D Viewport")
                        .Center()
                        .CloseButton(false)
                        .Floatable()
                        .MaximizeButton());

    // Add terminal as dockable pane (right side, floatable)
    m_auiManager.AddPane(m_pTerminal, wxAuiPaneInfo()
                        .Name("terminal")
                        .Caption("Terminal")
                        .Right()
                        .BestSize(400, 50)
                        .MinSize(250, 20)
                        .Floatable()
                        .CloseButton()
                        .MaximizeButton());

    // Commit all pane configurations
    m_auiManager.Update();

    // Add status bar with usage instructions
    CreateStatusBar();
    SetStatusText("Ready - Left mouse: rotate | Right mouse: pan | Scroll: zoom | Drag panes to dock/undock");
}

MainFrame::~MainFrame()
{
    // Clean up AUI manager
    m_auiManager.UnInit();
}

// Toolbar button handlers
void MainFrame::OnViewFront(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewFront();
    m_pTerminal->AppendOutput("View: Front (rotation reset, facing +Z)");
    m_pTerminal->ShowPrompt();
}

void MainFrame::OnViewTop(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewTop();
    m_pTerminal->AppendOutput("View: Top (looking down -Y axis)");
    m_pTerminal->ShowPrompt();
}

void MainFrame::OnViewSide(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewSide();
    m_pTerminal->AppendOutput("View: Side (looking from +X axis)");
    m_pTerminal->ShowPrompt();
}

void MainFrame::OnViewIsometric(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewIsometric();
    m_pTerminal->AppendOutput("View: Isometric (45° rotation on X and Y)");
    m_pTerminal->ShowPrompt();
}

void MainFrame::OnResetView(wxCommandEvent& event)
{
    m_pGlCanvas->ResetView();
    m_pTerminal->AppendOutput("View: Reset to default position");
    m_pTerminal->ShowPrompt();
}




