#include "frmscripteditor.h"

#include <sstream>
#include <codecvt>
#include <locale>
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>

#include "scriptnotebook.h"

#include "../icons/sz32/settings.xpm"
#include "../icons/sz32/python_logo.xpm"
#include "../icons/sz32/menu.xpm"

#include "../util/recentfiles.h"
#include "../util/cdate.h"

#include "../consts.h"



extern std::filesystem::path glbExeDir;




namespace scripting::editor
{

	frmScriptEditor::frmScriptEditor(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Script Editor")
	{
		SetIcon(python_logo_xpm);

		m_mgr = new wxAuiManager(this, wxAUI_MGR_LIVE_RESIZE);

		/******* start Settings, Recent Files, Previous Session Files   *****/
		m_RecentFiles = std::make_unique<util::CRecentFiles>(glbExeDir / consts::HOME / ".script"/ "recentfiles.txt");
		m_RecentFiles->ReadOrCreate();
		
		m_PrevSesFiles = std::make_unique<util::CRecentFiles>(glbExeDir / consts::HOME / ".script" /"previoussession.txt");
		m_PrevSesFiles->ReadOrCreate();


		/************* Load Directory List   ***********/
		m_DefDirPath = std::make_unique<util::CRecentFiles>(glbExeDir / consts::HOME / ".script"/ "defaultdir.txt");
		m_DefDirPath->ReadOrCreate();
		auto DefPathList = m_DefDirPath->GetList();

		m_DirList = new script::CScriptDirCtrl(this, DefPathList.size() > 0 ? *DefPathList.begin() : "");
		m_DirList->ShowHidden(false);


		/**   Load Script Notebook and any files from previous session *************/
		m_ScriptNtbk = new ScriptNtbk(this);
		LoadPreviouslyOpenedScripts();
		LoadDraftScripts();

		//Do not let other instances of ScienceSuit to open previoussession scripts
		std::filesystem::remove(glbExeDir / consts::HOME / ".script" / "previoussession.txt");

		if (m_ScriptNtbk->GetPageCount() == 0)
			m_ScriptNtbk->AddScript("");


		m_OutputWnd = new wxRichTextCtrl(this);


		m_AUIToolBar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_TEXT | wxAUI_TB_VERTICAL);
		m_AUIToolBar->Bind(wxEVT_UPDATE_UI, [&](wxUpdateUIEvent& event)
		{
			m_AUIToolBar->EnableTool(ID_RUN, m_ScriptNtbk->GetPageCount() > 0);
			event.Skip();
		});
		
		auto Menu = m_AUIToolBar->AddTool(ID_TOOLMENU, "Menu", wxBitmap(menu_xpm));
		auto RunTool = m_AUIToolBar->AddTool(ID_RUN, "Run", wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE));
		auto DirList = m_AUIToolBar->AddTool(ID_VIEW_DIRLIST, "Show", wxArtProvider::GetBitmap(wxART_NEW_DIR), "", wxITEM_CHECK);
		m_AUIToolBar->Realize();


		
		m_mgr->AddPane(m_AUIToolBar, wxAuiPaneInfo().Left().Resizable(false).CloseButton(false).Layer(1).Position(1));

		m_mgr->AddPane(m_ScriptNtbk, wxAuiPaneInfo().
			Name(GetPaneName(Pane::ScriptNotebook)).Caption("Script Notebook").
			Center().CloseButton(false).MaximizeButton(true).Layer(0).Position(1));

		m_mgr->AddPane(m_DirList, wxAuiPaneInfo().
			Name(GetPaneName(Pane::DirList)).Caption("Directory").Right().
			CloseButton(false).Show(false).MaximizeButton(true).Layer(0).Position(0));

		m_mgr->AddPane(m_OutputWnd, wxAuiPaneInfo().
			Name(GetPaneName(Pane::OutputWindow)).Caption("Output Window").
			Bottom().CloseButton(true).MaximizeButton(true).Layer(0).Position(1));
		
		m_ScriptNtbk->SetFocus();

		m_mgr->Update();


		Bind(wxEVT_ACTIVATE, &frmScriptEditor::OnActivate, this);
		Bind(wxEVT_CLOSE_WINDOW, &frmScriptEditor::OnClose, this);

		m_DirList->Bind(wxEVT_COMMAND_TREE_ITEM_ACTIVATED, &frmScriptEditor::OnTreeItemActivated, this);		

		m_OutputWnd->Bind(wxEVT_KEY_UP, [this](wxKeyEvent& event)
		{
			if (event.ControlDown() && event.GetKeyCode() == 'C' && wxTheClipboard->Open())
			{
				wxTheClipboard->SetData(new wxTextDataObject(m_OutputWnd->GetStringSelection()));
				wxTheClipboard->Close();
			}
		});


		Bind(wxEVT_COMMAND_TOOL_CLICKED, &frmScriptEditor::OnRun, this, ID_RUN);
		Bind(wxEVT_COMMAND_TOOL_CLICKED, &frmScriptEditor::OnToolMenuClicked, this, ID_TOOLMENU);
		Bind(wxEVT_COMMAND_TOOL_CLICKED, &frmScriptEditor::OnDirectoryView, this, ID_VIEW_DIRLIST);


		//Main Menu
		m_MainMenu = new wxMenu();
		wxMenuItem* Item = nullptr;

		Item = m_MainMenu->Append(ID_NEW, "New");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));

		m_MainMenu->AppendSeparator();

		Item = m_MainMenu->Append(ID_SAVE, "Save");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

		Item = m_MainMenu->Append(ID_SAVEALL, "Save All");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));

		m_MainMenu->AppendSeparator();

		Item = m_MainMenu->Append(ID_VIEW_WS, "Show Whitespace and TAB", "", wxITEM_CHECK);
		Item = m_MainMenu->Append(ID_VIEW_EOL, "Show End of Line", "", wxITEM_CHECK);

		m_MainMenu->AppendSeparator();

		Item = m_MainMenu->Append(ID_OPEN_FILE, "Open...");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN));

		m_MainMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& event) 
		{
			m_ScriptNtbk->AddScript("");
		},  ID_NEW);

		m_MainMenu->Bind(wxEVT_MENU, [this](wxCommandEvent& event)
		{
			m_ScriptNtbk->GetCurrentScript()->Save();
		}, ID_SAVE);

		m_MainMenu->Bind(wxEVT_MENU, &frmScriptEditor::OnFile_SaveAll, this, ID_SAVEALL);
		m_MainMenu->Bind(wxEVT_MENU, &frmScriptEditor::OnOpen, this, ID_OPEN_FILE);
		m_MainMenu->Bind(wxEVT_MENU, &frmScriptEditor::OnView_Whitespace_EOL, this, ID_VIEW_WS);
		m_MainMenu->Bind(wxEVT_MENU, &frmScriptEditor::OnView_Whitespace_EOL, this, ID_VIEW_EOL);
	}


	frmScriptEditor::~frmScriptEditor()
	{
		m_mgr->UnInit();
	}


	void frmScriptEditor::OnToolMenuClicked(wxCommandEvent& event)
	{	
		if (m_RecentFilesMenu)
			m_MainMenu->Destroy(ID_RECENTFILES);

		m_RecentFilesMenu = new wxMenu();

		for (const auto& path : m_RecentFiles->GetList())
		{
			if (!std::filesystem::exists(path))
				continue;

			int ID = wxNewId();

			m_RecentFilesMenu->Append(ID, path.wstring());
			m_RecentFilesMenu->Bind(wxEVT_MENU, [this, path](wxCommandEvent& CmdEvt)
			{
				try
				{
					OpenScriptFile(path);
				}
				catch (const std::exception& e)
				{
					wxMessageBox(e.what());
				}
			}, ID);
		}

		m_MainMenu->Append(ID_RECENTFILES,"Recent Files", m_RecentFilesMenu);
		
		m_MainMenu->Enable(ID_SAVE, m_ScriptNtbk->GetPageCount() > 0);
		m_MainMenu->Enable(ID_SAVEALL, m_ScriptNtbk->GetPageCount() > 1);
		m_MainMenu->Enable(ID_VIEW_WS, m_ScriptNtbk->GetPageCount() > 0);
		m_MainMenu->Enable(ID_VIEW_EOL, m_ScriptNtbk->GetPageCount() > 0);

		if (auto CurScript = m_ScriptNtbk->GetCurrentScript())
		{
			m_MainMenu->Enable(ID_SAVE, CurScript->IsModified() || CurScript->GetPath().empty());
			m_MainMenu->Check(ID_VIEW_WS, CurScript->GetViewWhiteSpace());
			m_MainMenu->Check(ID_VIEW_EOL, CurScript->GetViewEOL());
		}

		PopupMenu(m_MainMenu);
	}



	void frmScriptEditor::OnActivate(wxActivateEvent& event)
	{
		if (!event.GetActive())
			return;

		wxPosition LastViewPos;

		if (!m_ScriptNtbk)
		{
			event.Skip();
			return;
		}

		auto CurScript = m_ScriptNtbk->GetCurrentScript();
		if (!CurScript)
		{
			event.Skip();
			return;
		}


		size_t N = m_ScriptNtbk->GetPageCount();
		for (size_t i = 0; i < N; ++i)
		{
			auto script = (ScriptCtrl*)m_ScriptNtbk->GetPage(i);
			if (!script)
				continue;

			if (script->GetPath().empty())
				continue;

			wxDateTime ScriptLastSaved = script->GetLastSaved();
			if (!ScriptLastSaved.IsValid())
				return;

			wxStructStat strucStat;
			wxStat(script->GetPath().wstring(), &strucStat);
			auto filelen = strucStat.st_size;
			wxDateTime FileLastSaved(strucStat.st_mtime); //could be external

			if (!FileLastSaved.IsValid())
				return;

			if (FileLastSaved.IsLaterThan(ScriptLastSaved))
				script->LoadFile(script->GetPath());
		}

		event.Skip();
	}



	void frmScriptEditor::OnClose(wxCloseEvent& event)
	{
		m_PrevSesFiles->ClearList();

		for (size_t i = 0; i < m_ScriptNtbk->GetPageCount(); ++i) 
		{
			auto Script = (ScriptCtrl*)m_ScriptNtbk->GetPage(i);
			auto Path = Script->GetPath();
			if (!Path.empty())
			{
				if (!std::filesystem::exists(Path))
					continue;

				if (Script->IsModified())
					Script->Save();
			}
		}

		SaveSessionScripts();

		try 
		{
			if (m_DefDirPath)
			{
				m_DefDirPath->ClearList();
				m_DefDirPath->Append(m_DirList->GetDefaultPath().ToStdWstring());
				m_DefDirPath->Write();
			}

			if (m_RecentFiles)
				m_RecentFiles->Write();

			if (m_PrevSesFiles)
				m_PrevSesFiles->Write();
		}
		catch (const std::exception& e){
			wxMessageBox(e.what());
		}

		event.Skip();
	}

	 

	void frmScriptEditor::OnTreeItemActivated(wxTreeEvent& event)
	{
		wxFileName fileName(m_DirList->GetFilePath());
		wxString ext = fileName.GetExt();

		if (ext == "py")
			OpenScriptFile(m_DirList->GetFilePath().ToStdWstring());
		else
			event.Skip();
	}


	void frmScriptEditor::OnOpen(wxCommandEvent& event)
	{
		wxFileDialog dlg(this,
			"Open Script", wxEmptyString, wxEmptyString,
			"Python File (*.py)|*.py", wxFD_OPEN);

		if (dlg.ShowModal() != wxID_OK)
			return;

		try
		{
			wxString FilePath = dlg.GetPath();
			OpenScriptFile(FilePath.ToStdWstring());
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what());
		}
	}



	void frmScriptEditor::OnFile_SaveAll(wxCommandEvent& event)
	{
		size_t PageCount = m_ScriptNtbk->GetPageCount();

		for (size_t i = 0; i < PageCount; ++i)
		{
			ScriptCtrl* script = (ScriptCtrl*)m_ScriptNtbk->GetPage(i);

			/*
				Only save the scripts those are changed
				Empty scripts such as Script 1 will not be saved until it has been changed.
			*/
			if (script->IsModified())
				script->Save();
		}
	}


	void frmScriptEditor::OnView_Whitespace_EOL(wxCommandEvent& event)
	{
		auto curScript = m_ScriptNtbk->GetCurrentScript();

		auto Checked = event.IsChecked();
		int ID = event.GetId();

		if(ID == ID_VIEW_WS)
			curScript->SetViewWhiteSpace(Checked);
		else if(ID == ID_VIEW_EOL)
			curScript->SetViewEOL(Checked);
	}



	ScriptCtrl* frmScriptEditor::GetCurrentScript() const
	{
		return m_ScriptNtbk->GetCurrentScript();
	}

	

	void frmScriptEditor::OnRun(wxCommandEvent& event)
	{
		ScriptCtrl* curScript = m_ScriptNtbk->GetCurrentScript();
		assert(curScript != nullptr);

		ShowOutputPane();

		util::CDate date;
		auto currentTime = date.GetDate("", true) + " " + date.GetTime(":");

		m_OutputWnd->Clear();
		m_OutputWnd->AppendText("Time: " + currentTime);


		/*
			If the script already has a valid path then we save it and then proceed to run it
			Otherwise, a script file is created under INSTALLDIR/temp and it is run and then deleted
		*/

		PyObject* ModuleObj = curScript->GetPythonModule();
		m_stdOutErrCatcher.SetModule(ModuleObj);
		m_stdOutErrCatcher.StartCatching();

		auto Path = curScript->GetPath();
		if (Path.empty())
			PyRun_SimpleString(curScript->GetText().mb_str(wxConvUTF8));

		else
		{
			//does the script still exist?
			if (!std::filesystem::exists(Path))
			{
				m_OutputWnd->AppendText(Path.wstring() + L" does not exist!");
				return;
			}

			//has it been modified and NOT saved yet
			if (curScript->IsModified())
				curScript->Save();

			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			if (auto cp = _Py_wfopen(Path.wstring().c_str(), L"rb"))
				PyRun_SimpleFileEx(cp, converter.to_bytes(Path.wstring()).c_str(), true);

			//initial script path might have changed if script did not have a full path 
			m_OutputWnd->AppendText(" ( " + Path.wstring() + " )");
		}

		m_OutputWnd->Newline();

		std::wstring StdIOErr;
		m_stdOutErrCatcher.CaptureOutput(StdIOErr);
		m_stdOutErrCatcher.RestorePreviousIO();

		m_OutputWnd->AppendText(StdIOErr);
	}


	void frmScriptEditor::OnDirectoryView(wxCommandEvent& event)
	{
		m_AUIToolBar->SetToolLabel(ID_VIEW_DIRLIST, event.IsChecked() ? "Hide" : "Show");
		m_mgr->GetPane(GetPaneName(Pane::DirList)).Show(!m_mgr->GetPane(GetPaneName(Pane::DirList)).IsShown());

		m_mgr->Update();
	}


	void frmScriptEditor::ShowOutputPane(bool MakeItFloat)
	{
		auto& OutPane = m_mgr->GetPane(GetPaneName(Pane::OutputWindow));

		wxSize MinPaneSize = OutPane.min_size;
		wxSize frmSize = GetSize();
		
		if (MakeItFloat && !OutPane.IsFloating())
		{
			OutPane.FloatingSize(m_OutputWnd->GetSize().GetWidth(), frmSize.GetHeight() * 0.30);
			OutPane.Show();

			wxSize Sz = OutPane.floating_size;
			wxPoint pos = GetRect().GetBottomRight();
			pos.x -= Sz.GetWidth();
			pos.y -= Sz.GetHeight();

			OutPane.floating_pos = pos;
			OutPane.Float();
		}
		else if(!MakeItFloat)
		{
			int Cur_H = m_OutputWnd->GetSize().GetHeight();

			if (!OutPane.IsShown())
			{
				OutPane.Show();
				OutPane.MinSize(m_OutputWnd->GetSize().GetWidth(), Cur_H);
			}
			else
			{
				//If the current height of the output window (therefore pane) is less than 1% frame's height resize it
				if (Cur_H < frmSize.GetHeight() * 0.01)
					OutPane.MinSize(m_OutputWnd->GetSize().GetWidth(), frmSize.GetHeight() * 0.25);
			}
		}

		m_mgr->Update();

		//Restore the minsize so that user can resize it smaller than the min size
		OutPane.MinSize(MinPaneSize);
	}


	
	void frmScriptEditor::LoadDraftScripts()
	{
		auto UnsavedDir = glbExeDir / "home" / ".script" / "drafts";

		for (const auto& DirEntry : std::filesystem::directory_iterator(UnsavedDir))
		{
			if (DirEntry.is_directory())
				continue;

			auto Script = OpenScriptFile(DirEntry, false);
			Script->SetPath(DirEntry);
		}
	}



	void frmScriptEditor::LoadPreviouslyOpenedScripts()
	{
		if (!m_PrevSesFiles)
			return;

		auto FileList = m_PrevSesFiles->GetList();
		if (FileList.size() == 0)
			return;

		for (const auto& File : FileList)
		{
			//maybe the file has been deleted or moved
			if (!std::filesystem::exists(File))
				continue;

			//dont add to recent files menu
			OpenScriptFile(File, false);
		}
	}


	void frmScriptEditor::SaveSessionScripts()
	{
		util::CDate date;
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

		for (size_t i = 0; i < m_ScriptNtbk->GetPageCount(); ++i) 
		{
			auto Script = (ScriptCtrl*)m_ScriptNtbk->GetPage(i);
			if (!Script->GetPath().empty())
			{
				auto Path = Script->GetPath();
				auto ParentPath = Path.parent_path();

				if (ParentPath == (glbExeDir / consts::HOME / L".script" / L"drafts"))
					continue;

				m_PrevSesFiles->Append(Path);
				continue;
			}
			
			try
			{
				if (Script->GetText().empty())
					continue;

				std::string Date = date.GetDate("", true)+ " " + date.GetTime("");
				std::wstring FileName = Script->GetTitle().ToStdWstring() + L" - " + converter.from_bytes(Date) + L".py";
				Script->SaveStyledText(glbExeDir / consts::HOME / L".script" / L"drafts" / FileName);
			}
			catch (const std::exception&)
			{
				continue;
			}
		}
	}




	ScriptCtrl* frmScriptEditor::OpenScriptFile(const std::filesystem::path& Path, bool AddToRecentMenu)
	{
		//checks if there is already from the same path is open
		auto script = m_ScriptNtbk->AddScript(Path);
		if (!script)
			return nullptr;

		script->LoadFile(Path);

		//Dont undo a newly opened file as it will delete everything
		script->EmptyUndoBuffer();

		//Nothing edited yet
		script->DiscardEdits();

		if (AddToRecentMenu) 
		{
			if (m_RecentFiles)
				m_RecentFiles->Append(Path);
		}

		return script;
	}


}

