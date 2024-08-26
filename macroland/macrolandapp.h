#pragma once

#include <filesystem>

#include <wx/app.h>
#include <wx/cmdline.h>

class frmMacroLand;

class MacroLandApp : public wxApp
{
public:
	//core, stats, plot, roots ... modules
	void InitSciSuitModules();

protected:
	virtual bool OnInit() override;
	virtual int FilterEvent(wxEvent &event) override;

private:
	std::filesystem::path m_PyHome{};
	frmMacroLand *m_frmMacroLand{nullptr};
};


wxDECLARE_APP(MacroLandApp);

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
  {wxCMD_LINE_PARAM, NULL, NULL, "", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE},
  {wxCMD_LINE_NONE},
};

