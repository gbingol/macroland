#include "scriptnotebook.h"

#include <wx/artprov.h>
#include <wx/mimetype.h>

#include "frmscripteditor.h"
#include "scriptctrl.h"



namespace scripting::editor
{

	ScriptNtbk::ScriptNtbk(frmScriptEditor* const parent) :
		wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE), m_ParentFrame{ parent }
	{
		m_NBlankScripts = 0;

		Bind(wxEVT_AUINOTEBOOK_BUTTON, &ScriptNtbk::OnNotebookCloseButton, this);
		Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &ScriptNtbk::OnNotebookPageChanged, this);
		Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &ScriptNtbk::OnNotebookPageChanging, this);
		Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_DOWN, &ScriptNtbk::OnNotebookTabRightDown, this);
	}


	ScriptNtbk::~ScriptNtbk() = default;


	void ScriptNtbk::OnNotebookCloseButton(wxAuiNotebookEvent& event)
	{
		RemoveScript(m_curScript);

		if (GetPageCount() == 0)
			m_curScript = nullptr;
	}


	void ScriptNtbk::OnNotebookPageChanged(wxAuiNotebookEvent& evt)
	{
		int sel = GetSelection();
		if (sel >= 0)
			m_curScript = (ScriptCtrl*)GetPage(sel);
		else
			m_curScript = nullptr;
	}


	void ScriptNtbk::OnNotebookPageChanging(wxAuiNotebookEvent& evt)
	{
		int sel = GetSelection();
		if (sel >= 0)
			m_curScript = (ScriptCtrl*)GetPage(sel);
		else
			m_curScript = nullptr;

		if (m_curScript)
			m_curScript->HideAutoComplete();

		evt.Skip();
	}


	void ScriptNtbk::OnNotebookTabRightDown(wxAuiNotebookEvent& evt)
	{
		wxMenu Menu;
		wxMenuItem* Item = nullptr;

		//If it is NOT a draft script
		if (m_curScript && !m_curScript->GetPath().empty())
		{
			Item = Menu.Append(ID_OPENPATH, "Open in File Explorer");
			Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN));

			Item = Menu.Append(ID_OPENDEFEDITOR, "Open with Default Editor");
			Item->SetBitmap(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE));

			Menu.AppendSeparator();

			Item = Menu.Append(ID_DELSCRIPT, "Delete File and  Close Page");
			Item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));

			Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_OPENPATH);
			Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_OPENDEFEDITOR);
			Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_DELSCRIPT);

			Menu.AppendSeparator();
		}
		
		Item = Menu.Append(ID_CLOSE, "Close");	
		Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_CLOSE);

		if (GetPageCount() > 1){
			Menu.Append(ID_CLOSEALLBUTHIS, "Close all but this");
			Menu.Append(ID_CLOSEALL, "Close all");

			Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_CLOSEALLBUTHIS);
			Menu.Bind(wxEVT_MENU, &ScriptNtbk::OnTabMenu, this, ID_CLOSEALL);
		}

		PopupMenu(&Menu);

		if (GetPageCount() == 0)
			m_curScript = nullptr;
	}



	ScriptCtrl* ScriptNtbk::AddScript(const std::filesystem::path& FullPath)
	{
		wxString WndName; //text to show on the tab

		//check if a script with the full path is already open
		if (!FullPath.empty())
		{
			if (auto PageNum = FindScript(FullPath)) 
			{
				SetSelection(PageNum.value());
				return nullptr;
			}

			WndName = FullPath.filename().wstring();
		}
		else
			WndName = "Script" + std::to_string(++m_NBlankScripts);


		auto script = new ScriptCtrl(this);
		script->SetTitle(WndName);

		if (!AddPage(script, WndName, true))
			return nullptr;

		script->SetSavePoint();

		m_curScript = script; //current script
		script->SetTitle(WndName);

		if (!FullPath.empty())
			script->SetPath(FullPath);


		int PgNum = GetPageCount() == 0 ? 0 : GetPageCount() - 1;

		if (!FullPath.empty())
			SetPageToolTip(PgNum, FullPath.wstring());
		else
			SetPageToolTip(PgNum, WndName);

		return script;
	}


	bool ScriptNtbk::RemoveScript(ScriptCtrl* script, bool ForceClose)
	{
		for (size_t i = 0; i < GetPageCount(); i++)
		{	
			if ((ScriptCtrl*)GetPage(i) == script)
			{
				if (RemoveScript(i, ForceClose))
					return true;

				return false;
			}
		}

		return false;
	}



	int ScriptNtbk::RemoveScript(size_t id, bool ForceClose)
	{
		auto script = (ScriptCtrl*)GetPage(id);

		if (script->IsModified() && !ForceClose)
		{
			wxString ScrFullPath = script->GetPath().wstring();

			if (ScrFullPath == wxEmptyString)
				ScrFullPath = script->GetName();

			wxString msg = "You have not saved " + ScrFullPath + " yet. Do you want to save?";
			int confirm = wxMessageBox(msg, "Confirm", wxYES_NO | wxCANCEL);
			if (confirm == wxYES)
			{
				//If user cancels then save returns false
				if (!script->Save())
					return wxCANCEL;
			}
			else if (confirm == wxCANCEL)
				return wxCANCEL;
			else
				confirm = wxNO;

			//wxNO
			DeletePage(id);

			return confirm;
		}

		//If the script is not dirty it is equal to not saving (wxNO)
		DeletePage(id);

		return wxNO;
	}



	bool ScriptNtbk::RemoveAllScripts(bool ForceClose)
	{
		size_t pageNumber = 0;
		do 
		{
			int answer = RemoveScript(pageNumber, ForceClose);
			if (answer == wxCANCEL)
				return false;

		} while (GetPageCount() > 0);

		return true;
	}


	std::optional<size_t> ScriptNtbk::FindScript(const ScriptCtrl* script)
	{
		size_t PageCount = GetPageCount();

		for (size_t pageNum = 0; pageNum < PageCount; ++pageNum)
		{
			if ((ScriptCtrl*)GetPage(pageNum) == script)
				return std::optional<size_t>{pageNum};
		}

		return std::optional<size_t>();
	}


	std::optional<size_t> ScriptNtbk::FindScript(const std::filesystem::path& FullPath)
	{
		size_t PageCount = GetPageCount();

		for (size_t pageNum = 0; pageNum < PageCount; ++pageNum){
			ScriptCtrl* script = (ScriptCtrl*)GetPage(pageNum);

			if (script && script->GetPath() == FullPath)
				return std::optional<size_t>{pageNum};
		}

		return std::optional<size_t>();
	}


	void ScriptNtbk::OnTabMenu(wxCommandEvent& evt)
	{
		int evtId = evt.GetId();

		if (evtId == ID_OPENPATH)
		{
			wxString cmd = "explorer /select, " + m_curScript->GetPath().wstring();
			wxExecute(cmd, wxEXEC_ASYNC, NULL);
		}

		else if(evtId == ID_OPENDEFEDITOR)
		{
			std::unique_ptr<wxFileType> c_type(wxTheMimeTypesManager->GetFileTypeFromExtension("py"));

			wxString command = c_type->GetOpenCommand(m_curScript->GetPath().wstring());
			if (command.empty())
			{
				wxMessageBox("There is no default program to open this type of file.");
				return;
			}

			wxExecute(command);
		}

		else if (evtId == ID_DELSCRIPT)
		{
			auto Path = m_curScript->GetPath();

			if (!std::filesystem::exists(Path))
			{
				wxMessageBox("File at: " + Path.wstring() + " does not exist!", "ERROR");
				return;
			}

			int Ans = wxMessageBox("Delete file: " + Path.wstring(), "CONFIRM", wxYES_NO);
			if (Ans == wxNO)
				return;

			//remove the file
			std::filesystem::remove(Path);

			//close the page (forced close)
			RemoveScript(m_curScript, true);
		}

		else if (evtId == ID_CLOSEALLBUTHIS)
		{
			/*
				Delete the 0th pages until the current page
				If current page is 0th page, delete the 1st page until only 1 page left
			*/
			size_t pageNumber = 0;
			auto curScript = m_curScript;

			do
			{
				auto script = (ScriptCtrl*)GetPage(pageNumber);

				if (script == 0)
					break;

				if (script == curScript){
					pageNumber++;
					continue;
				}

				if (!RemoveScript(pageNumber))
					pageNumber++;

			} while (pageNumber < GetPageCount());

		}

		else if (evtId == ID_CLOSEALL)
		{
			bool ForceClose = false;

			RemoveAllScripts(ForceClose);

			m_ParentFrame->Close();
		}

		else if (evtId == ID_CLOSE)
		{
			RemoveScript(m_curScript);

			if (GetPageCount() == 0)
				m_ParentFrame->Close();
		}
	}

}