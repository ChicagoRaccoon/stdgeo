// Application implementation: creates and displays main window
#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(App);  // Macro to define main() and create App instance

bool App::OnInit() {
    MainFrame* frame = new MainFrame("StdGeo OpenGL Viewer");
    frame->Show(true);
    return true;  // true = continue running, false = exit immediately
}