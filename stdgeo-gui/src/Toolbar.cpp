
/**
 * THEORY OF OPERATION
 * 
 * This module manages the toolbar.
 */

#include "CommonHeader.h"

#include "Toolbar.h"
#include <wx/artprov.h>
#include <vector>


Toolbar::Toolbar(wxWindow* parent, GLCanvas* pGlCanvas, Terminal* pTerminal)
    : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER),
    m_pGlCanvas(pGlCanvas),
    m_pTerminal(pTerminal)
{
    AddTool(ID_VIEW_FRONT    , "Front", wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR), "View from front");
    AddTool(ID_VIEW_TOP      , "Top"  , wxArtProvider::GetBitmap(wxART_GO_UP     , wxART_TOOLBAR), "View from top"  );
    AddTool(ID_VIEW_SIDE     , "Side" , wxArtProvider::GetBitmap(wxART_GO_BACK   , wxART_TOOLBAR), "View from side" );
    AddTool(ID_VIEW_ISOMETRIC, "Iso"  , wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR), "Isometric view" );
    AddSeparator();
    AddTool(ID_VIEW_RESET    , "Reset", wxArtProvider::GetBitmap(wxART_UNDO      , wxART_TOOLBAR), "Reset view"     );
    Realize();
}

Toolbar::~Toolbar() 
{
    // Noop
}



// Toolbar button handlers
void Toolbar::OnViewFront(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewFront();
    m_pTerminal->AppendOutput("View: Front (rotation reset, facing +Z)");
    m_pTerminal->ShowPrompt();
}

void Toolbar::OnViewTop(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewTop();
    m_pTerminal->AppendOutput("View: Top (looking down -Y axis)");
    m_pTerminal->ShowPrompt();
}

void Toolbar::OnViewSide(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewSide();
    m_pTerminal->AppendOutput("View: Side (looking from +X axis)");
    m_pTerminal->ShowPrompt();
}

void Toolbar::OnViewIsometric(wxCommandEvent& event)
{
    m_pGlCanvas->SetViewIsometric();
    m_pTerminal->AppendOutput("View: Isometric (45° rotation on X and Y)");
    m_pTerminal->ShowPrompt();
}

void Toolbar::OnResetView(wxCommandEvent& event)
{
    m_pGlCanvas->ResetView();
    m_pTerminal->AppendOutput("View: Reset to default position");
    m_pTerminal->ShowPrompt();
}


