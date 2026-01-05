#pragma once

#include "CommonHeader.h"

#include <wx/wx.h>


class App : public wxApp
{
public:
    virtual bool OnInit() override;  // Called on application startup
};

