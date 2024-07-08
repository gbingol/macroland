#pragma once

#include <filesystem>

#include <wx/app.h>
#include <wx/cmdline.h>

 
class PyPkgMngrApp : public wxApp
{
protected:
	virtual bool OnInit() override; 
};


wxDECLARE_APP(PyPkgMngrApp);

