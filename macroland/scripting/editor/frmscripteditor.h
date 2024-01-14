#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include <script/directoryctrl.h>
#include <script/scripting_funcs.h>

#include "scriptctrl.h"



namespace util
{
	class CRecentFiles;
}



namespace scripting::editor
{

	class ScriptNtbk;
	class frmScriptEditor : public wxFrame
	{
		friend class ScriptCtrl;
	public:

		frmScriptEditor(wxWindow* parent);
		~frmScriptEditor();


		ScriptCtrl* OpenScriptFile(
			const std::filesystem::path& Path, //full path
			bool AddToRecentMenu = true);

		ScriptNtbk* const GetScriptNotebook() const{
			return m_ScriptNtbk;
		}


		ScriptCtrl* GetCurrentScript() const;


	protected:
		void OnActivate(wxActivateEvent& event);

		void OnClose(wxCloseEvent& event);

		void OnTreeItemActivated(wxTreeEvent& event);

		void OnRun(wxCommandEvent& event);

		void OnDirectoryView(wxCommandEvent& event);

		void OnToolMenuClicked(wxCommandEvent& event);
		void OnOpen(wxCommandEvent& event);
		void OnFile_SaveAll(wxCommandEvent& event);
		void OnView_Whitespace_EOL(wxCommandEvent& event);


	private:
		void ShowOutputPane(bool MakeItFloat = false);
		
		//The scripts which have a valid path (already been saved)
		void LoadPreviouslyOpenedScripts();

		void LoadDraftScripts();

		void SaveSessionScripts();


	protected:
		ScriptNtbk* m_ScriptNtbk = nullptr;
		pnlFindReplace* m_pnlFindRep = nullptr;
		wxMenu* m_RecentFilesMenu = nullptr;
		wxMenu* m_MainMenu{ nullptr };

		wxAuiToolBar* m_AUIToolBar;

	private:
		enum class Pane 
		{
			DirList = 0,
			ScriptNotebook,
			OutputWindow,
		};


		wxString GetPaneName(Pane pane) {
			wxString PaneNames[] = { "DirList", "ScriptNotebook", "OutputWindow"};

			return PaneNames[(int)pane];
		}

		const int ID_TOOLMENU{ wxNewId() };
		const int ID_NEW = wxNewId();
		const int ID_OPEN_FILE = wxNewId();
		const int ID_SAVE = wxNewId();
		const int ID_SAVEALL = wxNewId();
		const int ID_RECENTFILES = wxNewId();
		const int ID_RECENTFILE_MENUITEM = wxNewId();
		const int ID_RUN = wxNewId();

		const int ID_SCRIPTCLOSE = wxNewId();

		const int ID_UNDO = wxNewId();
		const int ID_REDO = wxNewId();
		const int ID_COPY = wxNewId();
		const int ID_CUT = wxNewId();
		const int ID_PASTE = wxNewId();
		const int ID_FIND = wxNewId();
		const int ID_REPLACE = wxNewId();

		const int ID_VIEW_WS = wxNewId();
		const int ID_VIEW_EOL = wxNewId();

		const int ID_REVEALINEXPLORER = wxNewId();
		const int ID_SETFOLDERDEFAULT = wxNewId();
		const int ID_VIEW_DIRLIST = wxNewId();


		script::CStdOutErrCatcher m_stdOutErrCatcher;

		//default path for directory control
		std::unique_ptr<util::CRecentFiles> m_DefDirPath;

		//files appended to recent files menu
		std::unique_ptr<util::CRecentFiles> m_RecentFiles;

		//previous session
		std::unique_ptr<util::CRecentFiles> m_PrevSesFiles; 


		wxAuiManager* m_mgr;

		script::CScriptDirCtrl* m_DirList;
		wxRichTextCtrl* m_OutputWnd;
	};

}