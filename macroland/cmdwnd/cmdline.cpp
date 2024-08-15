#include "cmdline.h"

#include <wx/sstream.h>
#include <wx/artprov.h>
#include <wx/gdicmn.h> 
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/file.h>

#include <lua.hpp>
#include "../lua/luautil.h"

#include <boost/json/value.hpp>
#include <boost/json/array.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include "../python/PythonWrapper.h"

#include "scripting_funcs.h"

#include "../consts.h"




extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;


static wxString CmdListtoCmd(const std::list<wxString>& lst)
{
	wxString retStr;
	for (const auto& cmd : lst)
		retStr += cmd + "\n";

	retStr.RemoveLast(1);

	return retStr;
}


namespace cmdedit
{
	
	wxDEFINE_EVENT(ssEVT_SCRIPTCTRL_RETURN, wxCommandEvent);


	COutputWnd::COutputWnd(wxWindow* parent,
		wxWindowID id,
		CStyledTextCtrl* InputWnd) : CStyledTextCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
	{
		m_PrntWnd = parent;
		m_InputWnd = InputWnd;

		//Since the goal is NOT to edit (this is output wnd), no need to show extra characters and bloat output
		SetViewEOL(false);
		SetViewWhiteSpace(false);

		//when number of outputs increase showing code folding makes the appearance rather confusing
		SetMarginWidth(MARGIN_FOLD, 0);

		SetUseHorizontalScrollBar(false);

		Bind(wxEVT_STC_MODIFIED, &COutputWnd::OnModified, this);
		Bind(wxEVT_RIGHT_UP, &COutputWnd::OnRightUp, this);

		Bind(wxEVT_SET_FOCUS, &COutputWnd::OnSetFocus, this);
		Bind(wxEVT_KILL_FOCUS, &COutputWnd::OnKillFocus, this);

		m_PrntWnd->Bind(wxEVT_SIZE, [&](wxSizeEvent& event)
			{
				wxStyledTextEvent evt;
				evt.SetEventType(wxEVT_STC_MODIFIED);
				wxPostEvent(this, evt);

				event.Skip();
			});
	}


	wxSize COutputWnd::DoGetBestSize() const
	{
		wxSize sz = GetBestSize();
		return sz;
	}


	void COutputWnd::AppendOutput(const wxString& txt, bool PrependLine)
	{
		if (!GetSelectedText().empty())
			SelectNone();

		if (!GetText().empty() && PrependLine)
			AppendLine();

		AppendText(txt);

		if (!IsShown())
			Show();
	}


	void COutputWnd::OnModified(wxStyledTextEvent& event)
	{
		wxSize szPrnt = m_PrntWnd->GetClientSize();

		int Line_H = TextHeight(0);
		int NLines = GetLineCount();

		if (NLines == 0)
		{
			Hide();
			return;
		}

		int Height = Line_H * NLines;
		int InputWnd_H = 0;
		if (m_InputWnd)
			InputWnd_H = m_InputWnd->FromDIP(m_InputWnd->TextHeight(0));

		int line = NLines;
		while ((Height + InputWnd_H) >= szPrnt.y && line <= NLines)
			Height = Line_H * (line--);

		int W = szPrnt.x;

		SetSize(wxSize(W, Height));
		ShowPosition(GetLastPosition());
	}


	void COutputWnd::OnSetFocus(wxFocusEvent& event)
	{
		wxClientDC dc(this);
		wxSize szTxt = dc.GetMultiLineTextExtent(GetText());

		wxFont fnt = GetFont();
		auto px = fnt.GetPixelSize();
		int pt = fnt.GetPointSize();

		SetScrollWidth(szTxt.x * px.y / pt);
		SetUseHorizontalScrollBar(true);

		event.Skip();
	}

	void COutputWnd::OnKillFocus(wxFocusEvent& event)
	{
		SetUseHorizontalScrollBar(false);
		event.Skip();
	}


	void COutputWnd::OnRightUp(wxMouseEvent& event)
	{
		wxMenu menu;

		wxString Lbl = m_ShowLineNo ? wxString("Hide Line Numbers") : wxString("Show Line Numbers");

		auto Item = menu.Append(ID_SHOWLINENO, Lbl);
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_REFRESH));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_SHOWLINENO);

		Item = menu.Append(ID_SAVE, "Save");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_SAVE);

		if (!GetSelectedText().empty())
		{
			Item = menu.Append(ID_COPY, "Copy");
			Item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
			menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_COPY);
		}

		menu.AppendSeparator();

		Item = menu.Append(ID_DELALL, "Delete All");
		Item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &COutputWnd::OnPopMenu, this, ID_DELALL);

		PopupMenu(&menu);
	}


	void COutputWnd::OnPopMenu(wxCommandEvent& event)
	{
		int evtID = event.GetId();

		if (evtID == ID_COPY)
		{
			wxString Txt = GetSelectedText();
			if (wxTheClipboard->Open()) 
			{
				wxTheClipboard->SetData(new wxTextDataObject(Txt));
				wxTheClipboard->Close();
			}
		}
		else if (evtID == ID_DELALL)
		{
			ClearAll();
		}
		else if (evtID == ID_SHOWLINENO)
		{
			SetMarginWidth(MARGIN_LINENUM, m_ShowLineNo ? 0 : TextWidth(wxSTC_STYLE_LINENUMBER, "9999"));
			m_ShowLineNo = !m_ShowLineNo;
		}
		else if (evtID == ID_SAVE)
		{
			wxFileDialog dlgSave(this, "Save contents", "", "", "*.py|*.py", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			int resp = dlgSave.ShowModal();

			if (resp == wxID_OK)
			{
				try
				{
					std::filesystem::path Path = dlgSave.GetPath().ToStdWstring();
					SaveStyledText(Path);
				}
				catch (const std::exception& e)
				{
					wxMessageBox(e.what());
				}
			}
		}

	}






	/************************************************************************/

	CInputWnd::CInputWnd(CCmdLine* parent, PyObject* Module) :
		wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBorder::wxBORDER_NONE)
	{
		m_ParentWnd = parent;
		m_PyModule = Module;

		OpenReadHist();
		m_HistPos = m_CmdHist.size();

		m_StTxt = new wxStaticText(this, wxID_ANY, ">>", wxDefaultPosition, wxDefaultSize, wxBorder::wxBORDER_NONE);
		m_StTxtDefBG = m_StTxt->GetBackgroundColour();

		m_Txt = new CStyledTextCtrl(this);
		m_Txt->SetUseHorizontalScrollBar(true);
		m_Txt->SetScrollWidth(10);
		m_Txt->SetMarginWidth(0, 0);//dont show line numbers
		m_Txt->SetMarginWidth(1, 0);//dont show marker margin
		m_Txt->SetMarginWidth(2, 0);//dont show fold margin

		SetBackgroundColour(wxColour(255, 255, 255));
		m_Txt->SetFont(wxFontInfo(12).FaceName("Consolas"));

		m_Txt->Bind(ssEVT_SCRIPTCTRL_RETURN, &CInputWnd::OnReturn, this);
		m_Txt->Bind(wxEVT_CHAR, &CInputWnd::OnChar, this);
		m_Txt->Bind(wxEVT_KEY_DOWN, &CInputWnd::OnKeyDown, this);
		m_Txt->Bind(wxEVT_KEY_UP, &CInputWnd::OnKeyUp, this);

		m_Txt->Bind(wxEVT_STC_MODIFIED, [this](wxStyledTextEvent& event)
		{
			/*
				Note that the parent of m_ParamsDoc is m_Txt, therefore the event propagates
				Thus we check the event object
			*/
			if(event.GetEventObject() == m_Txt)
			{
				if (event.GetLinesAdded() > 0 ) 
					SwitchToMultiMode();
			}
			
			event.Skip();
		});

		m_AutoComp = new AutoCompCtrl(m_Txt);
		m_ParamsDoc = new frmParamsDocStr(m_Txt);


		Bind(wxEVT_PAINT, &CInputWnd::OnPaint, this);
	}


	CInputWnd::~CInputWnd()
	{
		WriteCloseHist();
	}


	wxSize CInputWnd::DoGetBestSize() const
	{
		wxClientDC dc(const_cast<CInputWnd*> (this));

		wxCoord w = 0;
		wxCoord h = 0;
		dc.GetTextExtent(m_Txt->GetValue(), &w, &h);

		return wxSize(w, h);
	}


	void CInputWnd::OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);

		wxSize szClnt = GetClientSize();
		wxSize szStTxt = m_StTxt->GetSize();
		wxSize szTxt = wxSize(szClnt.x - szStTxt.x, szClnt.y);

		wxPoint TL = wxPoint(0, 0);

		m_StTxt->SetPosition(TL);

		m_Txt->SetSize(szTxt);
		m_Txt->SetPosition(wxPoint(TL.x + szStTxt.x, TL.y));
	}


	void CInputWnd::OnChar(wxKeyEvent &event)
	{
		int evtCode = event.GetKeyCode();
		int Pos = m_Txt->GetCurrentPos();

		m_Char = event.GetUnicodeKey(); 

		if(m_Char == '(' && m_Txt->GetStyleAt(Pos-1) != wxSTC_P_NUMBER)
		{
			wxString Word = m_Txt->GetPreviousWord(Pos);
			if(!Word.empty())
			{
				auto Params = GetfrmParamsDocStr(Word.ToStdString(wxConvUTF8), m_PyModule);
				if(!Params.Doc.empty() || !Params.Params.empty())
				{
					m_ParamsDoc->Show(std::make_pair(Params.Params, Params.Doc));
					m_AutoComp->Hide();
				}
			}
		}

		else if(m_Char == ')')
			m_ParamsDoc->Hide();

		event.Skip();
	}


	void CInputWnd::OnKeyDown(wxKeyEvent& evt)
	{
		int evtCode = evt.GetKeyCode();
		int KeyCode = evt.GetKeyCode();

		if (m_Mode == MODE::S && 
			(evtCode == WXK_UP || evtCode == WXK_DOWN) && 
			!m_AutoComp->IsShown())
		{
			if (m_CmdHist.size() == 0)
				return;

			evtCode == WXK_UP ? m_HistPos--: m_HistPos++;

			if (m_HistPos < 0 || m_HistPos >= m_CmdHist.size())
			{
				m_HistPos = std::clamp(m_HistPos, 0, (int)(m_CmdHist.size() - 1));
				wxBell();
				return;
			}

			const auto& CurCmd = m_CmdHist.at(m_HistPos);
			if (std::holds_alternative<wxString>(CurCmd))
				m_Txt->SetText(std::get<wxString>(CurCmd));
			else
			{
				m_Txt->SetText(CmdListtoCmd(std::get<std::list<wxString>>(CurCmd)));
				SwitchToSingleMode();
			}

			m_Txt->GotoPos(m_Txt->GetLastPosition());

			return;
		}

		else if (
			(evtCode == WXK_SPACE && evt.ControlDown()) || 
			(m_Mode == MODE::S && evtCode == WXK_TAB))
		{
			ShowAutoComp();
			return;
		}

		else if ((KeyCode == WXK_NUMPAD_ENTER || KeyCode == WXK_RETURN))
		{
			if (m_AutoComp->IsShown())
				m_AutoComp->Hide();
			
			bool SDown = evt.ShiftDown();
			bool Execute = (m_Mode == MODE::M && SDown) || (m_Mode == MODE::S && !SDown);
			if (Execute)
			{
				wxCommandEvent retEvt(ssEVT_SCRIPTCTRL_RETURN);
				retEvt.SetId(evt.GetId());
				retEvt.SetEventObject(evt.GetEventObject());

				wxPostEvent(m_Txt, retEvt);

				return;
			}
		}

		evt.Skip();
	}


	void CInputWnd::OnKeyUp(wxKeyEvent& evt)
	{
		int Pos = m_Txt->GetCurrentPos();
		
		if (m_Char == '.' && Pos>=2)
		{	
			int Style = m_Txt->GetStyleAt(Pos-2); //(Pos-1) = '.'
			if (Style == wxSTC_P_IDENTIFIER)
			{
				if(m_ParamsDoc->IsShown())
					m_ParamsDoc->Hide();
				ShowAutoComp();
			}	
		}

		m_Char = ' ';
		evt.Skip();
	}


	void CInputWnd::OnReturn(wxCommandEvent& evt)
	{
		auto OutWnd = m_ParentWnd->GetOutputWnd();

		if (!m_Txt->GetText().empty())
			m_Txt->GotoPos(m_Txt->GetLastPosition());

		auto CmdStr = m_Txt->GetText();
		CmdStr.Trim().Trim(false);

		if (CmdStr.empty())
		{
			OutWnd->AppendOutput(">>");
			//If there are multiple empty lines, all will be cleared
			m_Txt->SetText("");

			//Switch mode and adjust prompt sign
			SwitchToSingleMode();
			
			evt.Skip();
			return;
		}

		
		if (m_Mode == MODE::S)	
			m_CmdHist.push_back(CmdStr);
		else
		{	
			std::list<wxString> Cmds;
			for (size_t i = 0; i < m_Txt->GetLineCount(); ++i)
			{
				wxString curCmd = m_Txt->GetLineText(i);
				curCmd.Trim();
				Cmds.push_back(curCmd);
			}
			m_CmdHist.push_back(Cmds);
		}
		
		//Clear input so that line is always the zeroth
		m_Txt->SetText("");

		//Write to output the "raw" user command(s)
		OutWnd->AppendOutput(">>" + CmdStr);

		//Process Commands
		auto CmdOutput = ProcessCommand(CmdStr.utf8_str());

		//If any output write to output window
		if (!CmdOutput.empty())
			OutWnd->AppendOutput(CmdOutput);

		//reset history position to show the last (this) command
		m_HistPos = m_CmdHist.size();
		
		SwitchToSingleMode();

		evt.Skip();
	}


	
	wxString CInputWnd::ProcessCommand(const char* Cmd)
	{
		//ensure we have the GIL
		auto gstate = GILStateEnsure();

		PyObject* DictObj = PyModule_GetDict(m_PyModule);

		//string might contain UTF entries, so we encode it
		auto CodeObj = Py_CompileString(Cmd, "", m_Mode == MODE::M ? Py_file_input : Py_single_input);
		if (CodeObj)
		{
			auto EvalObj = PyEval_EvalCode(CodeObj, DictObj, DictObj);
			Py_DECREF(CodeObj);

			if (!EvalObj)
				PyErr_Print(); //if cannot evaluate (e.g. undeclared variable) print error to stream

			Py_XDECREF(EvalObj);
		}
		else
			PyErr_Print(); //if cannot compile (e.g. syntax error) print error so we can catch it


		PyObject* py_dict = PyModule_GetDict(m_PyModule);
		if (!py_dict)
			return {};

		auto catcher = PyDict_GetItemString(py_dict, "SYSCATCHSTDOUTPUT");
		if (!catcher)
			return {};

		auto OutputObj = PyObject_GetAttrString(catcher, "value");
		if (!OutputObj)
			return {};

		auto output = PyUnicode_AsWideCharString(OutputObj, nullptr);
		Py_DECREF(OutputObj);
		
		//reset "value", otherwise ouput's will accumulate and previous values will be printed each time
		PyObject_SetAttrString(catcher, "value", Py_BuildValue("s", ""));

		return output;
	}


	void CInputWnd::ShowAutoComp()
	{
		int pos = m_Txt->GetCurrentPos();
		if(pos == 0)
			return;

		auto word = m_Txt->GetPreviousWord(pos);
		if(!word.empty())
		{
			auto SymbolTbl = GetObjectElements(word.ToStdString(wxConvUTF8), m_PyModule);

			if (SymbolTbl.size() > 0)
			{
				m_AutoComp->Show(SymbolTbl);
				m_ParamsDoc->Hide();
			}
		}
	}


	void CInputWnd::SwitchToMultiMode()
	{
		m_Mode = MODE::M;
		m_StTxt->SetBackgroundColour(wxColour(0, 255, 0));
		m_StTxt->SetLabel("++");
	}

	void CInputWnd::SwitchToSingleMode()
	{
		m_Mode = MODE::S;
		m_StTxt->SetBackgroundColour(m_StTxtDefBG);
		m_StTxt->SetLabel(">>");
	}



	bool CInputWnd::OpenReadHist()
	{
		std::string HISTFILE = "home/cmdline_history.json";

		wxFile file;

		auto histPath = glbExeDir / HISTFILE;
		if (!std::filesystem::exists(histPath))
			file.Create(histPath.wstring(), true);
	
		bool IsOpened = file.Open(histPath.wstring(), wxFile::read);
		if (!IsOpened)
			return false;

		wxString JSON;
		file.ReadAll(&JSON, wxConvUTF8);
		file.Close();

		if (JSON.empty())
			return true;

		JSON = JSON.Trim().Trim(false);
		const char* UTFContent = JSON.mb_str(wxConvUTF8);

		boost::json::error_code ec;
		auto val = boost::json::parse(UTFContent, ec);

		if (ec.failed() || val.is_array() == false)
			return false;

		auto MainArr = val.as_array();
		for (size_t i = 0; i < MainArr.size(); ++i)
		{
			if (!MainArr[i].is_array())
				continue;

			auto Arr = MainArr[i].as_array();
			if (Arr.size() == 0)
				continue;

			if(Arr.size() == 1)
				m_CmdHist.push_back(wxString::FromUTF8(Arr[0].as_string().c_str()));
			else {
				std::list<wxString> lst;
				for (size_t j = 0; j < Arr.size(); ++j)
				{
					if (Arr[j].is_string())
						lst.push_back( wxString::FromUTF8(Arr[j].as_string().c_str()));
				}
				m_CmdHist.push_back(lst);
			}
		}

		return true;
	}


	bool CInputWnd::WriteCloseHist()
	{
		if (m_CmdHist.size() == 0)
			return 0;

		auto HISTFILE = "home/cmdline_history.json";
		wxFile file((glbExeDir / HISTFILE).wstring(), wxFile::write);

		boost::json::array MainArr;
		for (size_t i = 0; i < m_CmdHist.size(); ++i)
		{
			boost::json::array BoostArr;
			if (std::holds_alternative<std::list<wxString>>(m_CmdHist[i]))
			{
				const auto& Lst = std::get<std::list<wxString>>(m_CmdHist[i]);
				for (const auto& cmd : Lst)
					BoostArr.push_back((const char*)cmd.mb_str(wxConvUTF8));
			}
			else {
				auto str = std::get<wxString>(m_CmdHist[i]);
				BoostArr.push_back((const char*)str.mb_str(wxConvUTF8));
			}
			MainArr.push_back(BoostArr);
		}

		auto JSON = wxString::FromUTF8(boost::json::serialize(MainArr));
		return file.Write(JSON) && file.Close();
	}



	/**************************  CCmdLine  **********************************/

	CCmdLine::CCmdLine(wxWindow* parent, PyObject* Module) : 
		wxControl(parent, wxID_ANY)
	{
		m_PrntWnd = parent;

		SetBackgroundColour(wxColour(255, 255, 255));

		m_TxtInput = new CInputWnd(this, Module);
		m_TxtOutput = new COutputWnd(this, wxID_ANY, m_TxtInput->GetScriptCtrl());

		Bind(wxEVT_PAINT, &CCmdLine::OnPaint, this);
		Bind(wxEVT_SIZE, [&](wxSizeEvent& event)
		{
			m_TxtOutput->Refresh();
			m_TxtInput->Refresh();
			event.Skip();
		});
	}


	wxSize CCmdLine::DoGetBestSize() const
	{
		wxSize sz = m_TxtInput->GetBestSize();
		//wxSize sz2 = m_TxtOutput->GetBestSize();

		return sz;
	}


	void CCmdLine::OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
		wxSize szClnt = GetClientSize();

		wxRect OutputRect;
		wxPoint BL = wxPoint(0, 0);
		if (m_TxtOutput->GetTextLength() == 0)
			m_TxtOutput->Hide();
		else
		{
			m_TxtOutput->Show();
			OutputRect = m_TxtOutput->GetRect();
			BL = OutputRect.GetBottomLeft();
		}

		m_TxtInput->SetSize(wxSize(szClnt.x, szClnt.y - OutputRect.GetHeight()));
		m_TxtInput->SetPosition(BL);

		m_TxtOutput->Refresh();
		m_TxtInput->Refresh();
	}

}