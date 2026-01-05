/*
 * THEORY OF OPERATION
 * 
 * This file contains the main application implementation.
 * It creates and displays the main window.
*/

#include "CommonHeader.h"

#include "App.h"
#include "MainFrame.h"


// This macro defines main() and creates an instance of the App class.
wxIMPLEMENT_APP(App);

/**
 * This function provides the logical entry point to our application.
 * It creates the main frame and tells the wx framework to keep running.
 */
bool App::OnInit()
{
    // Create the main frame & specify its title
    MainFrame* pFrame = new MainFrame("StdGeo OpenGL Viewer");
    pFrame->Show(true);

    // true  : continue running
    // false : exit immediately
    return true;  
}

