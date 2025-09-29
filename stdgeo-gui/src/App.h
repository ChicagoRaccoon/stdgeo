// wxWidgets application entry point
#pragma once

#include <wx/wx.h>

class App : public wxApp {
public:
    virtual bool OnInit() override;  // Called on application startup
};