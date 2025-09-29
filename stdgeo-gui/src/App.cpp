#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    MainFrame* frame = new MainFrame("StdGeo OpenGL Viewer");
    frame->Show(true);
    return true;
}