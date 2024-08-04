#pragma once

#include <filesystem>
#include <stack>

#include <Python.h>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>

#include "styledtxtctrl.h"


#include "dllimpexp.h"



namespace script
{
	class CPopupHTML;
	class AutoCompCtrl;
	class AutoCompHelp;


	class CScriptCtrlBase :public CStyledTextCtrl
	{
	public:
		DLLSCRIPT CScriptCtrlBase(wxWindow* parent, PyObject* Module);

		virtual DLLSCRIPT ~CScriptCtrlBase();

		DLLSCRIPT void HideAutoComplete();

	protected:

		struct CompileError
		{
			//text showing where the error happened
			wxString Text = wxEmptyString;

			//line number of error
			int LineNo = -1;

			//message about error (i.e. syntaxerror....)
			wxString Msg = wxEmptyString;
		};


		DLLSCRIPT CompileError compile() const;

		DLLSCRIPT void OnKeyUp(wxKeyEvent& evt);
		DLLSCRIPT void OnCharAdded(wxStyledTextEvent& event);
		DLLSCRIPT void OnDwellStart(wxStyledTextEvent& event);
		DLLSCRIPT void OnDwellEnd(wxStyledTextEvent& event);
		DLLSCRIPT void OnModified(wxStyledTextEvent& event);
		DLLSCRIPT void OnAutoComp_EntrySelected(wxCommandEvent& event);

		DLLSCRIPT std::string CreateRandomModuleName(size_t N = 8) const;

	private:
		//uses indicator to show compile errors
		void CompileAndShowErrors();

		void ClearCompileErrors();

		//uses sys::wx::PopUpHTMLContent to show the error messages
		void PopupCompileErrorMessage(const wxPoint& ScreenCoord);

		//uses sys::wx::PopUpHTMLContent to show the docstring if available
		void PopupDocString(const wxPoint& ScreenCoord);


	protected:
		PyObject* m_PythonModule{ nullptr };

	private:

		AutoCompCtrl* m_AutoComp{ nullptr };
		AutoCompHelp* m_AutoCompHelp{ nullptr };


		//The identifier whose keys are shown at the auto-comp
		std::string m_Identifier_KeysShownOnAutoComp;

		const int INDICATOR = 3;
		const int INDICATOR_STYLE = wxSTC_INDIC_SQUIGGLE;

		CompileError m_CompileErr;

		CPopupHTML* m_frmInfo{ nullptr };
	};

}