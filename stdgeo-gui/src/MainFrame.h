#pragma once

#include <wx/wx.h>
#include "GLCanvas.h"

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);

private:
    GLCanvas* m_glCanvas;
    wxTextCtrl* m_textCtrl;
};