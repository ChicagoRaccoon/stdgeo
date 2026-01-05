
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
enum {
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
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800)),
      m_promptPos(0),
      m_historyIndex(-1)
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
    m_pTerminal = new wxTextCtrl(this, ID_TERMINAL, "",
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_RICH2 | wxTE_PROCESS_TAB);

    // Use monospace font for terminal feel
    wxFont terminalFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_pTerminal->SetFont(terminalFont);
    m_pTerminal->SetBackgroundColour(wxColour(0, 0, 0));
    m_pTerminal->SetForegroundColour(wxColour(0, 255, 0));

    // Bind key events for terminal behavior
    m_pTerminal->Bind(wxEVT_CHAR, &MainFrame::OnTerminalChar, this);

    // Print welcome message
    AppendOutput("=== StdGeo Terminal ===\n");
    AppendOutput("Commands: front, top, side, iso, reset, help");
    AppendOutput("Use Up/Down arrows for command history\n");
    ShowPrompt();

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
                        .BestSize(400, -1)
                        .MinSize(250, -1)
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

// Append text to terminal output
void MainFrame::AppendOutput(const wxString& text) {
    m_pTerminal->SetInsertionPointEnd();
    m_pTerminal->WriteText(text + "\n");
}

// Show command prompt at end of terminal
void MainFrame::ShowPrompt() {
    m_pTerminal->SetInsertionPointEnd();
    m_pTerminal->WriteText("> ");
    m_promptPos = m_pTerminal->GetInsertionPoint();
    m_pTerminal->SetInsertionPointEnd();
    m_pTerminal->SetFocus();
}

// Get text from current input line
wxString MainFrame::GetCurrentLine() {
    long lastPos = m_pTerminal->GetLastPosition();
    return m_pTerminal->GetRange(m_promptPos, lastPos);
}

// Clear current input line
void MainFrame::ClearCurrentLine() {
    long lastPos = m_pTerminal->GetLastPosition();
    m_pTerminal->Remove(m_promptPos, lastPos);
}

// Restrict editing to only the current input line
void MainFrame::RestrictEditableArea() {
    long insertPos = m_pTerminal->GetInsertionPoint();
    if (insertPos < m_promptPos) {
        m_pTerminal->SetInsertionPointEnd();
    }
}

// Toolbar button handlers
void MainFrame::OnViewFront(wxCommandEvent& event) {
    m_pGlCanvas->SetViewFront();
    AppendOutput("View: Front (rotation reset, facing +Z)");
    ShowPrompt();
}

void MainFrame::OnViewTop(wxCommandEvent& event) {
    m_pGlCanvas->SetViewTop();
    AppendOutput("View: Top (looking down -Y axis)");
    ShowPrompt();
}

void MainFrame::OnViewSide(wxCommandEvent& event) {
    m_pGlCanvas->SetViewSide();
    AppendOutput("View: Side (looking from +X axis)");
    ShowPrompt();
}

void MainFrame::OnViewIsometric(wxCommandEvent& event) {
    m_pGlCanvas->SetViewIsometric();
    AppendOutput("View: Isometric (45° rotation on X and Y)");
    ShowPrompt();
}

void MainFrame::OnResetView(wxCommandEvent& event) {
    m_pGlCanvas->ResetView();
    AppendOutput("View: Reset to default position");
    ShowPrompt();
}

// Terminal key event handler
void MainFrame::OnTerminalChar(wxKeyEvent& event) {
    int keyCode = event.GetKeyCode();

    RestrictEditableArea();

    if (keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER) {
        // Execute command on Enter
        wxString command = GetCurrentLine().Trim().Lower();

        m_pTerminal->WriteText("\n");

        if (!command.IsEmpty()) {
            // Add to history
            m_history.push_back(command);
            m_historyIndex = m_history.size();

            // Execute command
            ExecuteCommand(command);
        }

        ShowPrompt();
    }
    else if (keyCode == WXK_UP) {
        // Navigate history backward
        if (m_historyIndex > 0 && !m_history.empty()) {
            m_historyIndex--;
            ClearCurrentLine();
            m_pTerminal->WriteText(m_history[m_historyIndex]);
        }
    }
    else if (keyCode == WXK_DOWN) {
        // Navigate history forward
        if (!m_history.empty() && m_historyIndex < (int)m_history.size() - 1) {
            m_historyIndex++;
            ClearCurrentLine();
            m_pTerminal->WriteText(m_history[m_historyIndex]);
        }
        else if (m_historyIndex == (int)m_history.size() - 1) {
            m_historyIndex = m_history.size();
            ClearCurrentLine();
        }
    }
    else if (keyCode == WXK_BACK) {
        // Prevent backspace before prompt
        if (m_pTerminal->GetInsertionPoint() <= m_promptPos) {
            return;  // Don't process
        }
        event.Skip();
    }
    else if (keyCode == WXK_LEFT) {
        // Prevent moving cursor before prompt
        if (m_pTerminal->GetInsertionPoint() <= m_promptPos) {
            return;
        }
        event.Skip();
    }
    else if (keyCode == WXK_HOME) {
        // Home goes to start of input line
        m_pTerminal->SetInsertionPoint(m_promptPos);
    }
    else {
        // Allow other keys
        event.Skip();
    }
}

// Command execution
void MainFrame::ExecuteCommand(const wxString& command) {
    if (command == "front") {
        m_pGlCanvas->SetViewFront();
        AppendOutput("View: Front (rotation reset, facing +Z)");
    }
    else if (command == "top") {
        m_pGlCanvas->SetViewTop();
        AppendOutput("View: Top (looking down -Y axis)");
    }
    else if (command == "side") {
        m_pGlCanvas->SetViewSide();
        AppendOutput("View: Side (looking from +X axis)");
    }
    else if (command == "iso" || command == "isometric") {
        m_pGlCanvas->SetViewIsometric();
        AppendOutput("View: Isometric (45° rotation on X and Y)");
    }
    else if (command == "reset") {
        m_pGlCanvas->ResetView();
        AppendOutput("View: Reset to default position");
    }
    else if (command == "help") {
        AppendOutput("Available commands:");
        AppendOutput("  front      - View from front");
        AppendOutput("  top        - View from top");
        AppendOutput("  side       - View from side");
        AppendOutput("  iso        - Isometric view");
        AppendOutput("  reset      - Reset view to default");
        AppendOutput("  help       - Show this help message");
    }
    else {
        AppendOutput("Error: Unknown command '" + command + "'. Type 'help' for available commands.");
    }
}