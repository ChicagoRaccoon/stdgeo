
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

// TODO I don't know the best way or place to put these
// TODO I think I need to read a bit more about how best to register these
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_TOOL(ID_VIEW_FRONT    , Toolbar::OnViewFront    )
    EVT_TOOL(ID_VIEW_TOP      , Toolbar::OnViewTop      )
    EVT_TOOL(ID_VIEW_SIDE     , Toolbar::OnViewSide     )
    EVT_TOOL(ID_VIEW_ISOMETRIC, Toolbar::OnViewIsometric)
    EVT_TOOL(ID_VIEW_RESET    , Toolbar::OnResetView    )
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
{
    // Initialize AUI manager for dockable panes
    m_auiManager.SetManagedWindow(this);

    // Configure OpenGL context attributes (RGBA, double buffer, 16-bit depth)
    wxGLAttributes canvasAttrs;
    canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

    // Create OpenGL rendering canvas
    m_pGlCanvas = new GLCanvas(this, canvasAttrs);

    // Create terminal-style text control
    m_pTerminal = new Terminal(this, ID_TERMINAL, m_pGlCanvas);

    // Create toolbar with view preset buttons
    // TODO This toolbar should probably be created first?
    m_pToolBar = new Toolbar(this, m_pGlCanvas, m_pTerminal);

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



