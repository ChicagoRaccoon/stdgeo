
/**
 * THEORY OF OPERATION
 * 
 * This module manages the terminal.
 */

#include "CommonHeader.h"

#include "Terminal.h"
#include <wx/artprov.h>
#include <vector>

Terminal::Terminal(wxWindow* parent, wxWindowID windowId, GLCanvas* pGlCanvas)
    : wxTextCtrl(parent, windowId, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH2 | wxTE_PROCESS_TAB),
    m_promptPos(0),
    m_historyIndex(-1),
    m_pGlCanvas(pGlCanvas)
{
    // Use monospace font for terminal feel
    wxFont terminalFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    SetFont(terminalFont);
    SetBackgroundColour(wxColour(0, 0, 0));
    SetForegroundColour(wxColour(0, 255, 0));

    // Bind key events for terminal behavior
    Bind(wxEVT_CHAR, &Terminal::OnTerminalChar, this);

    // Print welcome message
    AppendOutput("=== StdGeo Terminal ===\n");
    AppendOutput("Commands: front, top, side, iso, reset, help");
    AppendOutput("Use Up/Down arrows for command history\n");
    ShowPrompt();
}

Terminal::~Terminal() 
{
    // Noop
}



// Append text to terminal output
void Terminal::AppendOutput(const wxString& text)
{
    SetInsertionPointEnd();
    WriteText(text + "\n");
}

// Show command prompt at end of terminal
void Terminal::ShowPrompt()
{
    SetInsertionPointEnd();
    WriteText("> ");
    m_promptPos = GetInsertionPoint();
    SetInsertionPointEnd();
    SetFocus();
}

// Get text from current input line
wxString Terminal::GetCurrentLine() 
{
    long lastPos = GetLastPosition();
    return GetRange(m_promptPos, lastPos);
}

// Clear current input line
void Terminal::ClearCurrentLine()
{
    long lastPos = GetLastPosition();
    Remove(m_promptPos, lastPos);
}

// Restrict editing to only the current input line
void Terminal::RestrictEditableArea()
{
    long insertPos = GetInsertionPoint();
    if (insertPos < m_promptPos)
    {
        SetInsertionPointEnd();
    }
}


// Terminal key event handler
void Terminal::OnTerminalChar(wxKeyEvent& event)
{
    int keyCode = event.GetKeyCode();

    RestrictEditableArea();

    if (keyCode == WXK_RETURN || keyCode == WXK_NUMPAD_ENTER)
    {
        // Execute command on Enter
        wxString command = GetCurrentLine().Trim().Lower();

        WriteText("\n");

        if (!command.IsEmpty())
        {
            // Add to history
            m_history.push_back(command);
            m_historyIndex = m_history.size();

            // Execute command
            ExecuteCommand(command);
        }

        ShowPrompt();
    }
    else if (keyCode == WXK_UP)
    {
        // Navigate history backward
        if (m_historyIndex > 0 && !m_history.empty())
        {
            m_historyIndex--;
            ClearCurrentLine();
            WriteText(m_history[m_historyIndex]);
        }
    }
    else if (keyCode == WXK_DOWN)
    {
        // Navigate history forward
        if (!m_history.empty() && m_historyIndex < (int)m_history.size() - 1)
        {
            m_historyIndex++;
            ClearCurrentLine();
            WriteText(m_history[m_historyIndex]);
        }
        else if (m_historyIndex == (int)m_history.size() - 1)
        {
            m_historyIndex = m_history.size();
            ClearCurrentLine();
        }
    }
    else if (keyCode == WXK_BACK)
    {
        // Prevent backspace before prompt
        if (GetInsertionPoint() <= m_promptPos)
        {
            return;  // Don't process
        }
        event.Skip();
    }
    else if (keyCode == WXK_LEFT)
    {
        // Prevent moving cursor before prompt
        if (GetInsertionPoint() <= m_promptPos)
        {
            return;
        }
        event.Skip();
    }
    else if (keyCode == WXK_HOME)
    {
        // Home goes to start of input line
        SetInsertionPoint(m_promptPos);
    }
    else
    {
        // Allow other keys
        event.Skip();
    }
}

// Command execution
void Terminal::ExecuteCommand(const wxString& command)
{
    if (command == "front")
    {
        m_pGlCanvas->SetViewFront();
        AppendOutput("View: Front (rotation reset, facing +Z)");
    }
    else if (command == "top")
    {
        m_pGlCanvas->SetViewTop();
        AppendOutput("View: Top (looking down -Y axis)");
    }
    else if (command == "side")
    {
        m_pGlCanvas->SetViewSide();
        AppendOutput("View: Side (looking from +X axis)");
    }
    else if (command == "iso" || command == "isometric")
    {
        m_pGlCanvas->SetViewIsometric();
        AppendOutput("View: Isometric (45° rotation on X and Y)");
    }
    else if (command == "reset")
    {
        m_pGlCanvas->ResetView();
        AppendOutput("View: Reset to default position");
    }
    else if (command == "help")
    {
        AppendOutput("Available commands:");
        AppendOutput("  front      - View from front");
        AppendOutput("  top        - View from top");
        AppendOutput("  side       - View from side");
        AppendOutput("  iso        - Isometric view");
        AppendOutput("  reset      - Reset view to default");
        AppendOutput("  help       - Show this help message");
    }
    else
    {
        AppendOutput("Error: Unknown command '" + command + "'. Type 'help' for available commands.");
    }
}


