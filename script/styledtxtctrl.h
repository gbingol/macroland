#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Python.h>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>

#include "dllimpexp.h"


DLLSCRIPT wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_LINEADDED, wxStyledTextEvent);


namespace script
{
	class CStyledTextCtrl : public wxStyledTextCtrl
	{
	public:
		struct INDICATORS
		{
			int FIND_UPANDDOWN = 1;
			int FIND_ALL = 0;
		};
	public:
		DLLSCRIPT CStyledTextCtrl(wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxAUI_NB_DEFAULT_STYLE,
			wxString WindowName = "");

		DLLSCRIPT ~CStyledTextCtrl();


		//writes text and (updates Current position and line and SavedPath member variables)
		DLLSCRIPT void SaveStyledText(const std::filesystem::path& FullPath);

		//Only writes text to the file
		DLLSCRIPT void WriteStyledText(const std::filesystem::path& FullPath);

		/*
			Note that although wxStyledTextCtrl has NewLine member func
			It seems problematic when the cursor is not at the end
			Even if the cursor moves to end using SetCursor the outcome is not the
			expected way. That's why AppendLine member func
		*/
		DLLSCRIPT void AppendLine();

		DLLSCRIPT wxString GetLineTextUntilCarret();


		//returns the contents of the line given by LineNo. If Trim=true then trims from both sides
		DLLSCRIPT wxString GetLine(int LineNo, bool Trim);

		//returns the contents of the line given by client coordinates
		DLLSCRIPT wxString GetLine(const wxPoint& point, bool Trim = true);

		//returns the identifier at a given point (client coords)
		DLLSCRIPT wxString GetWord(const wxPoint& point);


		//returns the contents of the line where caret is positioned
		DLLSCRIPT wxString GetCurLine(bool Trim);


		//Gets the range text by using GetLine method. The reason for writing is that GetRange method provided is not unicode friendly
		DLLSCRIPT wxString GetRangeText(long from, long to) const;

		//How many tabs and spaces make up the indentation: first int for number of tabs, second for spaces
		DLLSCRIPT std::pair<int, int> GetNumberOfTabsAndSpaces(int line_number) const;

		/*
			Find all matching SearchPhrase in Script subject to SearchFlagand
			Lexicals: 0 (All, meaning comments, keywords, etc...)
		*/
		DLLSCRIPT std::vector<int> FindAllText(
			const wxString& SearchPhrase,
			int SearchFlag = 0,
			int Lexicals = 0);

		DLLSCRIPT void HighlightPositions(std::vector<int> StartPosVec,
			int FillLength,
			int Indicator = 0,
			int IndicatorStyle = wxSTC_INDIC_ROUNDBOX);

		DLLSCRIPT void ClearFindIndicators();


	protected:
		DLLSCRIPT void OnSetFocus(wxFocusEvent& event);

		DLLSCRIPT void OnCharAdded(wxStyledTextEvent& event);
		DLLSCRIPT void OnNewLineAdded(wxStyledTextEvent& event);
		DLLSCRIPT void OnLeftUp(wxMouseEvent& event);

		/*
			Promotes the current line using tabs

			PromotionLevel=1, add one tab space to the previous line's indentation level
			PromotionLevel=0, keep the space+tab level as same as previous line
		*/
		DLLSCRIPT void PromoteCurLine(int PromotionLevel = 1);

		//Demote a Line
		DLLSCRIPT void DemoteLine(int lineNumber, int DemotionLevel = 1);


	private:
		void LoadDefaultStyle();


	protected:
		const int LINENUMBERMARGIN = 0;
		const int MARKERMARGIN = 1;
		const int FOLDMARGIN = 2;

		bool m_IsIndicatorOn = false;
		wxString m_LastWord;

	private:
		/*
			The following two are used to track the position of the view
			When the file is saved and the focus is lost to another view
			of program and the focus gained again, annoyingly the view
			was shifting to Line 0.
		*/
		int m_CurPos = -1;
		int m_CurVisibleLine = -1;

		//script has been explicitly saved to a path
		std::filesystem::path m_Path{};

		wxString m_Title{};
	};






	/**************************************************************** */



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