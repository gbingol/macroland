#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include <Python.h>

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/popupwin.h>



wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_LINEADDED, wxStyledTextEvent);


namespace cmdedit
{
	class CStyledTextCtrl : public wxStyledTextCtrl
	{
	public:
		CStyledTextCtrl(wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			wxString WindowName = "");

		~CStyledTextCtrl();


		//writes text and (updates Current position and line and SavedPath member variables)
		void SaveStyledText(const std::filesystem::path& FullPath);

		//Only writes text to the file
		void WriteStyledText(const std::filesystem::path& FullPath);

		/*
			Note that although wxStyledTextCtrl has NewLine member func
			It seems problematic when the cursor is not at the end
			Even if the cursor moves to end using SetCursor the outcome is not the
			expected way. That's why AppendLine member func
		*/
		void AppendLine();

		wxString GetLineTextUntilCarret();


		//returns the contents of the line given by LineNo. If Trim=true then trims from both sides
		wxString GetLine(int LineNo, bool Trim);

		//returns the contents of the line given by client coordinates
		wxString GetLine(const wxPoint& point, bool Trim = true);

		//returns the identifier at a given point (client coords)
		wxString GetWord(const wxPoint& point);

		//Get the word before Pos (including Pos). Word can be numpy or numpy.random
		wxString GetPreviousWord(int Pos);

		std::optional<size_t> GetMatchingBrace(int Pos);

		//returns the contents of the line where caret is positioned
		wxString GetCurLine(bool Trim);


		//How many tabs and spaces make up the indentation: first int for number of tabs, second for spaces
		std::pair<int, int> GetNumberOfTabsAndSpaces(int line_number) const;


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

	protected:
		const int MARGIN_LINENUM = 0;
		const int MARGIN_MARKER = 1;
		const int MARGIN_FOLD = 2;

		const int INDIC_BRACE = 40;
		const int INDIC_BRACEFILL = 41;
	};

}