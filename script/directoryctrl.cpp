#include "directoryctrl.h"

#include <map>

#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filefn.h> 

#include <wx/mimetype.h> //wxFileType



namespace script
{

	CScriptDirCtrl::CScriptDirCtrl(
		wxWindow * parent, 
		const std::filesystem::path& path)
	{
		Create(parent, wxID_ANY, path.wstring());	

		Bind(wxEVT_COMMAND_TREE_DELETE_ITEM, &CScriptDirCtrl::OnTreeDelItem, this);
		Bind(wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK, &CScriptDirCtrl::OnTreeItemRightClick, this);
		Bind(wxEVT_COMMAND_TREE_ITEM_ACTIVATED, &CScriptDirCtrl::OnTreeItemActivated, this);
	}


	CScriptDirCtrl::~CScriptDirCtrl() = default;

	void CScriptDirCtrl::OnTreeItemActivated(wxTreeEvent & event)
	{
		wxString SelectedPath = GetFilePath();

		wxFileName fileName(SelectedPath);
		wxString ext = fileName.GetExt();

		if (ext == "exe" || ext == "") 
		{
			event.Skip();
			return;
		}

		std::unique_ptr<wxFileType> c_type(wxTheMimeTypesManager->GetFileTypeFromExtension(ext));
		if (!c_type) {
			wxMessageBox("Unknown extension.");
			return;
		}

		wxString command = c_type->GetOpenCommand(GetFilePath());
		if (command.empty()) 
		{
			wxMessageBox("There is no default program to open this type of file.");
			return;
		}

		wxExecute(command);

		event.Skip();
	}


	void CScriptDirCtrl::OnTreeDelItem(wxTreeEvent & event) {}


	void CScriptDirCtrl::OnTreeItemRightClick(wxTreeEvent & event)
	{
		//Make sure right click also selects
		GetTreeCtrl()->SelectItem(event.GetItem());

		wxString SelectedPath = GetFilePath();

		wxFileName fileName(SelectedPath);	
		bool IsDir = fileName.GetExt() == "" ? true : false;

		wxMenu menu;
		menu.Append(ID_REVEALINEXPLORER, "Reveal in Explorer");
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &CScriptDirCtrl::OnRevealInExplorer, this, ID_REVEALINEXPLORER);

		if (IsDir) {
			menu.Append(ID_SETFOLDERDEFAULT, "Set as default");
			menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &CScriptDirCtrl::OnSetFolderDefault, this, ID_SETFOLDERDEFAULT);
		}

		GetTreeCtrl()->PopupMenu(&menu);

		event.Skip();
	}


	void CScriptDirCtrl::OnRevealInExplorer(wxCommandEvent & event)
	{
		wxString cmd = "explorer /select, " + GetPath();
		wxExecute(cmd, wxEXEC_ASYNC, NULL);
	}

	void CScriptDirCtrl::OnSetFolderDefault(wxCommandEvent & event)
	{
		SetDefaultPath(GetPath());
	}


}