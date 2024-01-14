#pragma once

#include <filesystem>

#include <wx/wx.h>
#include <wx/dirctrl.h>
#include <wx/aui/aui.h>
#include <wx/richtext/richtextctrl.h>

#include "dllimpexp.h"

namespace script
{

	class CScriptDirCtrl : public wxGenericDirCtrl
	{
		
	public:
		DLLSCRIPT CScriptDirCtrl(
			wxWindow * parent, 
			const std::filesystem::path& path);
		
		DLLSCRIPT ~CScriptDirCtrl();

	protected:

		DLLSCRIPT void OnTreeItemActivated(wxTreeEvent & event);
		DLLSCRIPT void OnTreeDelItem(wxTreeEvent& event);
		DLLSCRIPT void OnTreeItemRightClick(wxTreeEvent& event);

		DLLSCRIPT void OnRevealInExplorer(wxCommandEvent& event);
		DLLSCRIPT void OnSetFolderDefault(wxCommandEvent& event);

	private:	
		const int ID_REVEALINEXPLORER = wxNewId();
		const int ID_SETFOLDERDEFAULT = wxNewId();
	};

}