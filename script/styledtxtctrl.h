#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Python.h>

#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/stc/stc.h>
#include <wx/popupwin.h>

#include "dllimpexp.h"


DLLSCRIPT wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_LINEADDED, wxStyledTextEvent);


namespace script
{
	class CStyledTextCtrl : public wxStyledTextCtrl
	{
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

		//Get the word before Pos (including Pos). Word can be numpy or numpy.random
		DLLSCRIPT wxString GetPreviousWord(int Pos);

		//returns the contents of the line where caret is positioned
		DLLSCRIPT wxString GetCurLine(bool Trim);


		//How many tabs and spaces make up the indentation: first int for number of tabs, second for spaces
		DLLSCRIPT std::pair<int, int> GetNumberOfTabsAndSpaces(int line_number) const;


	protected:
		void OnCharAdded(wxStyledTextEvent& event);
		void OnNewLineAdded(wxStyledTextEvent& event);

		/*
			Promotes the current line using tabs

			PromotionLevel=1, add one tab space to the previous line's indentation level
			PromotionLevel=0, keep the space+tab level as same as previous line
		*/
		void PromoteCurLine(int PromotionLevel = 1);

		//Demote a Line
		void DemoteLine(int lineNumber, int DemotionLevel = 1);

		//Used for module
		std::string CreateRandoName(size_t N = 8) const;


	private:
		void LoadDefaultStyle();


	protected:
		const int LINENUMBERMARGIN = 0;
		const int MARKERMARGIN = 1;
		const int FOLDMARGIN = 2;

		const int INDIC_BRACE = 0;

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



}