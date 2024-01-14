#pragma once

#include <filesystem>
#include <stack>
#include <set>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>

#include <Python.h>

#include <script/scriptctrlbase.h>




wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_CARETPOSCHANGED, wxCommandEvent);





namespace scripting::editor
{

	class ScriptNtbk;
	class pnlFindReplace;


	class ScriptCtrl :public script::CScriptCtrlBase
	{
		struct Constants;

	public:
		ScriptCtrl(ScriptNtbk* parent);
		virtual ~ScriptCtrl();

		bool Save(const std::filesystem::path& FullPath);
		bool Save();


		//This returns the last time OnModified Event called
		wxDateTime GetLastSaved() const
		{
			return m_LastSaved;
		}

		void SetLastSaved(const wxDateTime& datetime){
			m_LastSaved = datetime;
		}


		PyObject* GetPythonModule();


	protected:
		void OnKeyUp(wxKeyEvent& evt);
		
		void OnModified(wxStyledTextEvent& event);

		void OnLeftDown(wxMouseEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnFocus(wxFocusEvent& event);
		void OnFocusLost(wxFocusEvent& event);

		void OnCaretPositionChanged(wxCommandEvent& event);

		void UpdateTabInfo();

	private:		
		const int STATBARFIELD = 1;
		wxDateTime m_LastSaved;
		bool m_IsIndicatorOn{ false };
		ScriptNtbk* m_ParentWindow{ nullptr };
	};


}