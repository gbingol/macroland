#pragma once

#include <optional>
#include <filesystem>

#include <wx/wx.h>
#include <wx/mdi.h>
#include <wx/aui/auibook.h>



namespace scripting::editor
{

	class frmScriptEditor;
	class ScriptCtrl;

	class ScriptNtbk : public wxAuiNotebook
	{
		friend class frmScriptEditor;


	public:
		ScriptNtbk(frmScriptEditor* const parent);

		virtual ~ScriptNtbk();

		std::optional<size_t> FindScript(const ScriptCtrl* script);
		std::optional<size_t> FindScript(const std::filesystem::path& FullPath);

		frmScriptEditor* const GetParent() const
		{
			return m_ParentFrame;
		}


		bool CloseAllScripts()
		{
			return RemoveAllScripts(false);
		}


		void ProcessNotebookPageChanged(wxAuiNotebookEvent& evt)
		{
			OnNotebookPageChanged(evt);
		}


	protected:
		void OnNotebookCloseButton(wxAuiNotebookEvent& event);
		void OnNotebookPageChanged(wxAuiNotebookEvent& evt);
		void OnNotebookPageChanging(wxAuiNotebookEvent& evt);
		void OnNotebookTabRightDown(wxAuiNotebookEvent& evt);

		void OnTabMenu(wxCommandEvent& evt);

	private:
		ScriptCtrl* AddScript(const std::filesystem::path& FullPath);

		//if ForceClose=true, then user will not have the chance to save the script even if it is dirty
		bool RemoveScript(ScriptCtrl* script, bool ForceClose = false);

		//returns wxNO, wxYES or wxCANCEL
		int  RemoveScript(size_t id, bool ForceClose = false);

		//true: all scripts have been closed
		bool RemoveAllScripts(bool ForceClose = false);


		ScriptCtrl* GetCurrentScript() const
		{
			return m_curScript;
		}

		int GetNumberOfBlankScripts() const
		{
			return m_NBlankScripts;
		}

		void ResetNumberofBlankScripts(){
			m_NBlankScripts = 0;
		}


	private:

		frmScriptEditor* m_ParentFrame = nullptr;
		ScriptCtrl* m_curScript = nullptr;

		//To track the number of opened scripts, it is always increments
		int m_NBlankScripts;

		const int ID_CLOSE = wxNewId();
		const int ID_CLOSEALLBUTHIS = wxNewId();
		const int ID_CLOSEALL = wxNewId();

		//open path in system's explorer
		const int ID_OPENPATH{ wxNewId() };

		//open in default editor
		const int ID_OPENDEFEDITOR{ wxNewId() };

		//delete the script and close the page
		const int ID_DELSCRIPT{ wxNewId() };
	};


}