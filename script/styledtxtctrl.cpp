#include "styledtxtctrl.h"

#include <sstream>
#include <wx/file.h>

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

		m_Path = "";

		SetBufferedDraw(true);
		StyleClearAll();
		SetLexer(wxSTC_LEX_PYTHON);

		LoadDefaultStyle();

		Bind(wxEVT_LEFT_UP, &CStyledTextCtrl::OnLeftUp, this);
		Bind(wxEVT_SET_FOCUS, &CStyledTextCtrl::OnSetFocus, this);
		Bind(wxEVT_STC_CHARADDED, &CStyledTextCtrl::OnCharAdded, this);
		Bind(ssEVT_SCRIPTCTRL_LINEADDED, &CStyledTextCtrl::OnNewLineAdded, this);

		SetUseAntiAliasing(true);
		SetSavePoint();
	}


	CStyledTextCtrl::~CStyledTextCtrl()
	{
		Unbind(wxEVT_LEFT_UP, &CStyledTextCtrl::OnLeftUp, this);
		Unbind(wxEVT_SET_FOCUS, &CStyledTextCtrl::OnSetFocus, this);
		Unbind(wxEVT_STC_CHARADDED, &CStyledTextCtrl::OnCharAdded, this);
		Unbind(ssEVT_SCRIPTCTRL_LINEADDED, &CStyledTextCtrl::OnNewLineAdded, this);
	}


	void CStyledTextCtrl::LoadDefaultStyle()
	{
		SetWordChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMONPQRSTUVWXYZ_");

		SetKeyWords(0, GetReservedWords());
		SetKeyWords(1, GetBuiltIns()); //Keywords

		SetVisiblePolicy(wxSTC_VISIBLE_SLOP, 3);

		SetCodePage(65001);

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

		//folding
		SetMarginWidth(FOLDMARGIN, 16);// -- fold margin
		SetMarginType(FOLDMARGIN, wxSTC_MARGIN_SYMBOL);
		SetMarginMask(FOLDMARGIN, wxSTC_MASK_FOLDERS);
		SetMarginSensitive(FOLDMARGIN, true);
		SetAutomaticFold(wxSTC_AUTOMATICFOLD_SHOW | wxSTC_AUTOMATICFOLD_CLICK | wxSTC_AUTOMATICFOLD_CHANGE);
		SetProperty("fold", "1");
		SetProperty("fold.compact", "1");
		SetProperty("fold.indentation", "1");
		SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

		SetMarginWidth(MARKERMARGIN, 16);
		SetMarginType(MARKERMARGIN, wxSTC_MARGIN_SYMBOL);
		SetMarginSensitive(MARKERMARGIN, true);

		MarkerDefine(1, wxSTC_MARK_ROUNDRECT, *wxWHITE, *wxRED);
		MarkerDefine(2, wxSTC_MARK_ARROW, *wxBLACK, *wxGREEN);


		wxColour grey(128, 128, 128);
		MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUSCONNECTED, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, *wxWHITE, grey);
		MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER, *wxWHITE, grey);


		wxColor BG = wxColor(255, 255, 255);
		wxFont Font = wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Consolas");

		auto IDs = { wxSTC_P_COMMENTBLOCK, wxSTC_P_COMMENTLINE, wxSTC_P_TRIPLEDOUBLE, wxSTC_P_STRING, wxSTC_P_CHARACTER,
		wxSTC_P_OPERATOR, wxSTC_P_IDENTIFIER, wxSTC_P_WORD, wxSTC_P_WORD2, wxSTC_P_DEFNAME,
		wxSTC_P_CLASSNAME, wxSTC_P_DECORATOR, wxSTC_P_NUMBER };

		for (auto id : IDs)
		{
			StyleSetBackground(id, BG);
			StyleSetFont(id, Font);
		}

		wxColor comments_FG = wxColor(127, 127, 127), comments_BG = BG;
		wxFont comments_Font = wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, false, "Consolas");

		wxColor strings_FG = wxColor(0, 127, 0);
		wxFont strings_Font = wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, false, "Consolas");

		wxColor ReservedWords_FG = wxColor(14, 1, 126);
		wxFont ReservedWords_Font = wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, "Consolas");


		StyleSetForeground(wxSTC_P_COMMENTBLOCK, comments_FG);
		StyleSetFont(wxSTC_P_COMMENTBLOCK, comments_Font);

		StyleSetForeground(wxSTC_P_COMMENTLINE, comments_FG);
		StyleSetFont(wxSTC_P_COMMENTLINE, comments_Font);

		//triple quote for __doc__
		StyleSetForeground(wxSTC_P_TRIPLEDOUBLE, comments_FG);
		StyleSetFont(wxSTC_P_TRIPLEDOUBLE, comments_Font);

		//double quote
		StyleSetForeground(wxSTC_P_STRING, strings_FG);
		StyleSetFont(wxSTC_P_STRING, strings_Font);

		//single quote
		StyleSetForeground(wxSTC_P_CHARACTER, strings_FG);
		StyleSetFont(wxSTC_P_CHARACTER, strings_Font);

		StyleSetForeground(wxSTC_P_OPERATOR, wxColor(0, 0, 0));
		StyleSetForeground(wxSTC_P_IDENTIFIER, wxColor(0, 0, 0));

		//reserved words
		StyleSetForeground(wxSTC_P_WORD, ReservedWords_FG);
		StyleSetFont(wxSTC_P_WORD, ReservedWords_Font);

		//builtins words
		StyleSetForeground(wxSTC_P_WORD2, wxColor(153, 153, 0));

		StyleSetForeground(wxSTC_P_DEFNAME, wxColor(70, 130, 180)); //steel blue
		StyleSetForeground(wxSTC_P_CLASSNAME, wxColor(32, 178, 170));
		StyleSetForeground(wxSTC_P_DECORATOR, wxColor(153, 153, 0)); //dark yellow
		StyleSetForeground(wxSTC_P_NUMBER, wxColor(127, 0, 0));

		

		SetCaretLineVisible(true);

		wxColor CaretLineBG = wxColor(230, 235, 235);
		SetCaretLineBackground(CaretLineBG);
	}



	void CStyledTextCtrl::OnSetFocus(wxFocusEvent& event)
	{
		if (m_CurVisibleLine > 0)
			ScrollToLine(m_CurVisibleLine);

		else if (m_CurPos >= 0)
		{
			int cur_line = LineFromPosition(m_CurPos);
			ScrollToLine(cur_line);
		}

		m_CurPos = -1;
		m_CurVisibleLine = -1;

		event.Skip();
	}


	void CStyledTextCtrl::OnLeftUp(wxMouseEvent& event)
	{
		int TextLen = GetText().Length();
		int curPos = GetCurrentPos();

		int curPosStyle = GetStyleAt(curPos);

		if (curPosStyle != wxSTC_P_IDENTIFIER)
		{
			IndicatorClearRange(0, GetTextLength());

			Refresh();
			event.Skip();

			return;
		}


		int posSt = WordStartPosition(curPos, true);
		int posEnd = WordEndPosition(posSt, true);

		//identifier
		wxString ID = GetRange(posSt, posEnd);

		//All start positions
		auto StAll = FindAllText(ID, wxSTC_FIND_WHOLEWORD);

		INDICATORS ind;
		if (StAll.size() > 0)
			HighlightPositions(StAll, ID.length(), ind.FIND_ALL);

		event.Skip();
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
			wxStyledTextEvent LineAdded(ssEVT_SCRIPTCTRL_LINEADDED);
			LineAdded.SetLine(event.GetLine());
			LineAdded.SetLinesAdded(event.GetLinesAdded());
			LineAdded.SetPosition(event.GetPosition());
			LineAdded.SetX(event.GetX());
			LineAdded.SetY(event.GetY());

			wxPostEvent(this, LineAdded);

			event.Skip();
			return;
		}


		if (m_IsIndicatorOn)
		{
			IndicatorClearRange(0, GetTextLength());
			m_IsIndicatorOn = false;
		}

		event.Skip();
	}


	void CStyledTextCtrl::HighlightPositions(
		std::vector<int> StartPosVec,
		int FillLength,
		int Indicator,
		int IndicatorStyle)
	{
		IndicatorClearRange(0, GetTextLength());

		Refresh();

		//style of the indicator
		SetIndicatorCurrent(Indicator);
		IndicatorSetStyle(Indicator, IndicatorStyle);

		for (auto& pos : StartPosVec)
			IndicatorFillRange(pos, FillLength);

		m_IsIndicatorOn = true;
	}


	void CStyledTextCtrl::ClearFindIndicators()
	{
		INDICATORS ind;
		SetIndicatorCurrent(ind.FIND_ALL);
		IndicatorClearRange(0, GetText().length());

		SetIndicatorCurrent(ind.FIND_UPANDDOWN);
		IndicatorClearRange(0, GetText().length());
	}



	void CStyledTextCtrl::SaveStyledText(const std::filesystem::path& FullPath)
	{
		m_CurPos = GetCurrentPos();
		m_CurVisibleLine = GetFirstVisibleLine();
		m_Path = FullPath;
		m_Title = FullPath.filename().wstring();

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


	void CStyledTextCtrl::LoadFile(const std::filesystem::path& FullPath)
	{
		if (!GetText().empty())
		{
			m_CurPos = GetCurrentPos();
			m_CurVisibleLine = GetFirstVisibleLine();
		}

		wxFile file;
		wxString Script;

		if (file.Open(FullPath.wstring()))
			file.ReadAll(&Script);
		else
		{
			file.Close();
			throw std::exception("Could not open file");
		}

		SetValue(Script);
		SetSavePoint();

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


	wxString CStyledTextCtrl::GetCurLine(bool Trim)
	{
		int cur_line = LineFromPosition(GetCurrentPos());

		if (cur_line < 0)
			return wxEmptyString;

		return GetLine(cur_line, Trim);
	}


	wxString CStyledTextCtrl::GetRangeText(long from, long to) const
	{
		wxString retStr;

		int LineStart = LineFromPosition(from);
		int LineEnd = LineFromPosition(to);

		for (int i = LineStart; i <= LineEnd; ++i)
		{
			wxString line = GetLineText(i);
			line.append("\r\n");

			retStr << line;
		}

		return retStr;
	}


	std::pair<int, int> CStyledTextCtrl::GetNumberOfTabsAndSpaces(int line_number) const
	{
		int LineIndentation = GetLineIndentation(line_number);
		int indentSize = GetIndent();

		int NumberOfTabs = LineIndentation / indentSize;
		int NumberOfSpaces = LineIndentation - NumberOfTabs * indentSize;

		return { NumberOfTabs, NumberOfSpaces };
	}


	std::vector<int> CStyledTextCtrl::FindAllText(
		const wxString& SearchPhrase,
		int SearchFlag,
		int Lexicals)
	{
		std::vector<int> PosVector;

		int ScriptLen = GetTextLength();
		int SearchTextLen = SearchPhrase.length();

		int WordStart = FindText(0, ScriptLen, SearchPhrase, SearchFlag);

		if (WordStart < 0)
			return PosVector;

		int WordStyle = GetStyleAt(WordStart);

		if (Lexicals == 0)
			PosVector.push_back(WordStart);

		else if (WordStyle == wxSTC_P_IDENTIFIER && Lexicals == wxSTC_P_IDENTIFIER)
			PosVector.push_back(WordStart);

		else if (WordStyle == wxSTC_P_STRING && Lexicals == wxSTC_P_STRING)
			PosVector.push_back(WordStart);


		int NIter = 0; //safeguard for infinite while iteration
		int MaxIter = ScriptLen - SearchTextLen;

		while (WordStart >= 0)
		{
			NIter++;

			//if the script is 1000 characters and identifier is 5 characters long, we should not do more than 200 iterations
			if (NIter > MaxIter)
				break;

			WordStart = FindText(WordStart + SearchTextLen, ScriptLen, SearchPhrase, SearchFlag);
			if (WordStart < 0)
				return PosVector;

			WordStyle = GetStyleAt(WordStart);

			if (Lexicals == 0)
				PosVector.push_back(WordStart);

			else if (WordStyle == wxSTC_P_IDENTIFIER && Lexicals == wxSTC_P_IDENTIFIER)
				PosVector.push_back(WordStart);

			else if (WordStyle == wxSTC_P_STRING && Lexicals == wxSTC_P_STRING)
				PosVector.push_back(WordStart);
		}

		return PosVector;
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