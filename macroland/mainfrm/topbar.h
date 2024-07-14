#pragma once

#include <filesystem>

#include <wx/wx.h>
#include <wx/hyperlink.h>

#include "../util_funcs.h"




class frmMacroLand;


class CTopBar : public wxPanel
{
public:

	CTopBar(frmMacroLand* parent,
		wxWindowID id = wxID_ANY);

	~CTopBar();


protected:
	void OnFile(wxHyperlinkEvent& event);
	void OnWindows(wxHyperlinkEvent& event);

	void OnOpen(wxCommandEvent& event);

private:
	//Using terminal command, opens the project file in a separate instance
	void ExecuteProjFile(const std::filesystem::path& ProjPath);

	void InitFileMenu();

private:
	frmMacroLand* m_ParentWnd{ nullptr };

	wxHyperlinkCtrl* m_File;
	wxHyperlinkCtrl* m_Windows;
	wxMenu* m_RecentProjMenu;
	wxMenu* m_FileMenu;

	const int ID_PROJ_SAVE = wxNewId();
	const int ID_PROJ_OPEN = wxNewId();
	const int ID_RECENTPROJ{ wxNewId() };
	const int ID_EXTMNGR{ wxNewId() };
	const int ID_FULLSCREEN{ wxNewId() };

	std::unique_ptr<util::CRecentFiles> m_RecentFiles;
};