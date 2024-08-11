#include "styledtxtctrl.h"

#include <sstream>
#include <chrono>
#include <random>

#include <wx/file.h>

#include "autocompletion.h"

#include "scripting_funcs.h"



static 	std::string GetReservedWords()
{
	std::stringstream ss;

	ss << "False break for  not None class from or ";
	ss << "True continue global pass __peg_parser__  def if raise ";
	ss << "and del import return as elif  in  try ";
	ss << "assert else is while async  except lambda with ";
	ss << "await finally  nonlocal yield";

	return ss.str();
}

static std::string GetBuiltIns()
{
	std::stringstream ss;

	ss << "ArithmeticError AssertionError AttributeError ";
	ss << "BaseException BlockingIOError BrokenPipeError BufferError BytesWarning ";
	ss << "ChildProcessError ConnectionAbortedError ConnectionError ConnectionRefusedError ConnectionResetError ";
	ss << "DeprecationWarning EOFError Ellipsis EncodingWarning EnvironmentError Exception ";
	ss << "False FileExistsError FileNotFoundError FloatingPointError FutureWarning ";
	ss << "GeneratorExit IOError ImportError ImportWarning IndentationError IndexError InterruptedError IsADirectoryError ";
	ss << "KeyError KeyboardInterrupt LookupError MemoryError ModuleNotFoundError ";
	ss << "NameError None NotADirectoryError NotImplemented NotImplementedError ";
	ss << "OSError OverflowError PendingDeprecationWarning PermissionError ProcessLookupError ";
	ss << "RecursionError ReferenceError ResourceWarning RuntimeError RuntimeWarning ";
	ss << "StopAsyncIteration StopIteration SyntaxError SyntaxWarning SystemError SystemExit ";
	ss << "TabError TimeoutError True TypeError ";
	ss << "UnboundLocalError UnicodeDecodeError UnicodeEncodeError UnicodeError UnicodeTranslateError UnicodeWarning UserWarning ";
	ss << "ValueError Warning WindowsError ZeroDivisionError ";
	ss << "__build_class__ __debug__ __doc__ __import__ __loader__ __name__ __package__ __spec__ ";
	ss << "abs aiter all anext any ascii bin bool breakpoint bytearray bytes callable chr classmethod compile complex copyright ";
	ss << "credits delattr dict dir divmod enumerate eval exec exit filter float format frozenset getattr globals hasattr hash ";
	ss << "help hex id input int isinstance issubclass iter len license list locals map max memoryview min next object oct ";
	ss << "open ord pow print property quit range repr reversed round set setattr slice sorted staticmethod str sum super tuple type vars zip";

	return ss.str();
}




wxDEFINE_EVENT(ssEVT_SCRIPTCTRL_LINEADDED, wxStyledTextEvent);

namespace script
{
	CStyledTextCtrl::CStyledTextCtrl(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		wxString WindowName) : wxStyledTextCtrl(parent, id, pos, size, style, WindowName)
	{
		SetBufferedDraw(true);
		StyleClearAll();
		SetLexer(wxSTC_LEX_PYTHON);

		SetWordChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMONPQRSTUVWXYZ_");
		SetKeyWords(0, GetReservedWords());
		SetKeyWords(1, GetBuiltIns()); //Keywords

		SetUseTabs(true);
		SetIndentationGuides(true);
		SetIndent(8);
		SetTabWidth(8);
		SetBackSpaceUnIndents(true);
		SetViewEOL(false);
		SetViewWhiteSpace(true);
		SetWhitespaceForeground(true, wxColor(128, 128, 128)); //dark gray
		SetTabDrawMode(wxSTC_TD_LONGARROW);

		SetMarginType(LINENUMBERMARGIN, wxSTC_MARGIN_NUMBER);
		SetMarginWidth(LINENUMBERMARGIN, TextWidth(wxSTC_STYLE_LINENUMBER, "9999"));// -- Line number margin

		IndicatorSetForeground(INDIC_BRACE, wxColor(0, 255, 0));


		wxFont Font = wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Consolas");

		auto IDs = { wxSTC_P_COMMENTBLOCK, wxSTC_P_COMMENTLINE, wxSTC_P_TRIPLEDOUBLE, 
		wxSTC_P_STRING, wxSTC_P_CHARACTER,
		wxSTC_P_OPERATOR, wxSTC_P_IDENTIFIER, wxSTC_P_WORD, 
		wxSTC_P_WORD2, wxSTC_P_DEFNAME,
		wxSTC_P_CLASSNAME, wxSTC_P_DECORATOR, wxSTC_P_NUMBER };

		for (auto id : IDs)
		{
			StyleSetBackground(id, wxColor(255, 255, 255));
			StyleSetFont(id, Font);
		}

		auto Cmnts = {wxSTC_P_COMMENTBLOCK, wxSTC_P_COMMENTLINE, wxSTC_P_TRIPLEDOUBLE};
		for(auto stc_p: Cmnts)
			StyleSetForeground(stc_p, wxColor(127, 127, 127));

		auto Strs = {wxSTC_P_STRING, wxSTC_P_CHARACTER};
		for(auto stc_str:Strs)
			StyleSetForeground(stc_str, wxColor(0, 127, 0));

		StyleSetForeground(wxSTC_P_OPERATOR, wxColor(0, 0, 0));
		StyleSetForeground(wxSTC_P_IDENTIFIER, wxColor(0, 0, 0));

		//reserved words
		StyleSetForeground(wxSTC_P_WORD, wxColor(14, 1, 126));
		StyleSetFont(wxSTC_P_WORD, Font.MakeBold());

		//builtins words
		StyleSetForeground(wxSTC_P_WORD2, wxColor(153, 153, 0));

		StyleSetForeground(wxSTC_P_DEFNAME, wxColor(70, 130, 180)); //steel blue
		StyleSetForeground(wxSTC_P_CLASSNAME, wxColor(32, 178, 170));
		StyleSetForeground(wxSTC_P_DECORATOR, wxColor(153, 153, 0)); //dark yellow
		StyleSetForeground(wxSTC_P_NUMBER, wxColor(127, 0, 0));

		SetCaretLineVisible(true);
		SetCaretLineBackground(wxColor(230, 235, 235));

		SetUseAntiAliasing(true);
		SetSavePoint();

		Bind(wxEVT_STC_CHARADDED, &CStyledTextCtrl::OnCharAdded, this);
		Bind(ssEVT_SCRIPTCTRL_LINEADDED, &CStyledTextCtrl::OnNewLineAdded, this);
	}


	CStyledTextCtrl::~CStyledTextCtrl()
	{
		Unbind(wxEVT_STC_CHARADDED, &CStyledTextCtrl::OnCharAdded, this);
		Unbind(ssEVT_SCRIPTCTRL_LINEADDED, &CStyledTextCtrl::OnNewLineAdded, this);
	}



	void CStyledTextCtrl::OnNewLineAdded(wxStyledTextEvent& event)
	{
		//Control line indentation
		int cur_pos = GetCurrentPos();
		int cur_line = LineFromPosition(cur_pos);

		//previous line's text
		wxString PrevTxt = GetLineText(cur_line - 1);

		if (PrevTxt.Contains(":") && PrevTxt.rfind(':') == PrevTxt.size() - 1)
			PromoteCurLine(1);

		else if (PrevTxt.Trim().Trim(false) == "pass")
			DemoteLine(cur_line);
		else
			PromoteCurLine(0);
	}


	void CStyledTextCtrl::OnCharAdded(wxStyledTextEvent& event)
	{
		int evtKey = event.GetKey();

		/*
		This is needed as when user presses enter then Windows adds \n\r characters and the OnCharAdded is called twice
		In this case we want it to be called only once, that's why we act on only for \r (wxSTC_EOL_CR)
		*/
		int new_line_key = (GetEOLMode() == wxSTC_EOL_CR) ? 13 : 10;

		if (evtKey == new_line_key)
		{
			wxStyledTextEvent LnAdd(ssEVT_SCRIPTCTRL_LINEADDED);
			LnAdd.SetLine(event.GetLine());
			LnAdd.SetLinesAdded(event.GetLinesAdded());
			LnAdd.SetPosition(event.GetPosition());
			LnAdd.SetX(event.GetX());
			LnAdd.SetY(event.GetY());

			wxPostEvent(this, LnAdd);
		}

		event.Skip();
	}



	void CStyledTextCtrl::SaveStyledText(const std::filesystem::path& FullPath)
	{
		WriteStyledText(FullPath);
		SetSavePoint();
	}


	void CStyledTextCtrl::WriteStyledText(const std::filesystem::path& FullPath)
	{
		wxFile file;

		wxString Script = GetText();
		if (bool IsOpened = file.Open(FullPath.wstring(), wxFile::write))
		{
			bool IsSaved = file.Write(Script);
			if (!IsSaved)
			{
				file.Close();
				throw std::exception("Could not successfully save the file");
			}
		}
		else
		{
			file.Close();
			throw std::exception("Cannot open the file.");
		}

		file.Close();
	}



	void CStyledTextCtrl::AppendLine()
	{
		int EOL = GetEOLMode();

		if (EOL == wxSTC_EOL_CR)
			AppendText("\r");
		else if (EOL == wxSTC_EOL_CRLF)
			AppendText("\r\n");
		else if (EOL == wxSTC_EOL_LF)
			AppendText("\n");
	}

	wxString CStyledTextCtrl::GetLineTextUntilCarret()
	{
		int cur_pos = GetCurrentPos();

		int cur_line = LineFromPosition(cur_pos);
		if (cur_line < 0) 
			return wxEmptyString;

		int curLinePos = PositionFromLine(cur_line);

		return GetTextRange(curLinePos, cur_pos);
	}


	wxString CStyledTextCtrl::GetLine(int LineNo, bool Trim)
	{
		if (!Trim)
			return wxStyledTextCtrl::GetLine(LineNo);

		return wxStyledTextCtrl::GetLine(LineNo).Trim().Trim(false);
	}


	wxString CStyledTextCtrl::GetLine(const wxPoint& point, bool Trim)
	{
		int Pos = PositionFromPoint(point);

		long Line, Col;
		PositionToXY(Pos, &Col, &Line);

		return GetLine(Line, Trim);
	}


	wxString CStyledTextCtrl::GetWord(const wxPoint& point)
	{
		int Pos = PositionFromPoint(point);
		int Start = WordStartPosition(Pos, true);
		int End = WordEndPosition(Pos, true);

		return GetRange(Start, End);
	}



	wxString CStyledTextCtrl::GetPreviousWord(int Pos)
	{
		auto WordChars = GetWordChars();

		wxString word;

		while(true)
		{
			auto c = GetTextRange(Pos-1, Pos);
			if(c.empty())
				break;

			if(WordChars.Contains(c) || c == ".")
				word.Prepend(c);
			else
				break;
			Pos--;
		}

		word = word.Trim().Trim(false);

		return word;
	}



	wxString CStyledTextCtrl::GetCurLine(bool Trim)
	{
		int cur_line = LineFromPosition(GetCurrentPos());

		if (cur_line < 0)
			return wxEmptyString;

		return GetLine(cur_line, Trim);
	}



	std::pair<int, int> CStyledTextCtrl::GetNumberOfTabsAndSpaces(int line_number) const
	{
		int LineIndentation = GetLineIndentation(line_number);
		int indentSize = GetIndent();

		int NumberOfTabs = LineIndentation / indentSize;
		int NumberOfSpaces = LineIndentation - NumberOfTabs * indentSize;

		return { NumberOfTabs, NumberOfSpaces };
	}


	
	void CStyledTextCtrl::PromoteCurLine(int PromotionLevel)
	{
		if (PromotionLevel < 0)
			return;

		int CurLinePos = LineFromPosition(GetCurrentPos());

		const auto [PrevTab, PrevSpace] = GetNumberOfTabsAndSpaces(CurLinePos - 1);

		int Target_Tabs, Target_Spaces;

		Target_Tabs = PrevTab + PromotionLevel;
		Target_Spaces = PrevSpace;

		wxString indentText;
		if (Target_Tabs > 0)
		{
			for (int i = 0; i < Target_Tabs; ++i)
				indentText.append('\t');
		}

		if (Target_Spaces > 0)
		{
			for (int i = 0; i < Target_Spaces; ++i)
				indentText.append(' ');
		}

		AddText(indentText);
	}


	void CStyledTextCtrl::DemoteLine(int lineNumber, int DemotionLevel)
	{
		//TODO: Here we assume that the line indentation is always with '\t', make sure it is so
		int pos = GetLineIndentPosition(lineNumber);
		DeleteRange(pos - 1, DemotionLevel);
	}
}