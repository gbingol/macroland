#pragma once

#include <string>
#include <filesystem>
#include <vector>

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/html/htmlwin.h>
#include <wx/listctrl.h>

#include "extension.h"


class frmExtensionMngr:public wxFrame
{
public:
	frmExtensionMngr(wxWindow* parent);
	~frmExtensionMngr() = default;

protected:
	void OnListEntrySelected(wxListEvent& event);
	void OnRightDown(wxMouseEvent& event);

	void OnInstall(wxCommandEvent& event);
	void OnExtensionMenu(wxCommandEvent& event); 

private:
	void SetupListView();
	void LoadExtensions();

protected:
	wxSplitterWindow* m_split;
	wxListView* m_LWExt{ nullptr };
	wxHtmlWindow* m_HTMLWin{ nullptr };

	const int ID_UNINSTALL{ wxNewId() };
	const int ID_DISABLE{ wxNewId() };
	const int ID_ENABLE{ wxNewId() };
	const int ID_SHOWINEXPLORER{ wxNewId() };

private:
	std::vector<extension::CExtension> m_Extensions;
	int m_SelIndex{ -1 };
};

