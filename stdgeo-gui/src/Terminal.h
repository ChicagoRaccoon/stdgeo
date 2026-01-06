

#pragma once

#include "CommonHeader.h"


#include <GL/glew.h>  // Must be included before other GL headers
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include "GLCanvas.h"

class Terminal : public wxTextCtrl
{
public:
    Terminal(wxWindow* parent, wxWindowID id, GLCanvas* pGlCanvas);
    ~Terminal();

    void  AppendOutput  (const wxString& text);  // Add text to terminal
    void  ShowPrompt    (void);

private:
    // Event handlers for terminal input
    void  OnTerminalChar  (wxKeyEvent& event);

    // Terminal helpers
    void      ExecuteCommand        (const wxString& command);
    wxString  GetCurrentLine        (void);
    void      ClearCurrentLine      (void);
    void      RestrictEditableArea  (void);

    // Terminal state
    long                   m_promptPos;     // Start position of current input
    std::vector<wxString>  m_history;       // Command history
    int                    m_historyIndex;  // Current position in history

    // TODO This variable should be removed and replaced with an actual event handler.
    GLCanvas*     m_pGlCanvas;   // 3D rendering viewport

};

