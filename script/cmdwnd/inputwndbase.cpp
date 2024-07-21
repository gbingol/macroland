#include "inputwndbase.h"



namespace script
{
	CInputWndBase::CInputWndBase(wxWindow* parent, PyObject* Module) :
		wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBorder::wxBORDER_NONE)
	{
		m_ParentWnd = parent;

		m_StTxt = new wxStaticText(this, wxID_ANY, ">>", wxDefaultPosition, wxDefaultSize, wxBorder::wxBORDER_NONE);
		m_StTxtDefBG = m_StTxt->GetBackgroundColour();

		m_Txt = new script::CScriptCtrlBase(this, nullptr);
		m_Txt->SetMarginWidth(0, 0);//dont show line numbers
		m_Txt->SetMarginWidth(1, 0);//dont show marker margin
		m_Txt->SetMarginWidth(2, 0);//dont show fold margin
		m_Txt->SetUseHorizontalScrollBar(false);

		SetBackgroundColour(wxColour(255, 255, 255));
		m_Txt->SetFont(wxFontInfo(12).FaceName("Consolas"));

		m_PyModule = Module;
		m_stdOutErrCatcher.SetModule(m_PyModule);

		if (!m_stdOutErrCatcher.StartCatching())
			wxMessageBox("Internal error, cannot capture io.");

		Bind(wxEVT_PAINT, &CInputWndBase::OnPaint, this);

		m_Txt->Bind(wxEVT_KEY_UP, &CInputWndBase::OnKeyUp, this);
		m_Txt->Bind(wxEVT_KEY_DOWN, &CInputWndBase::OnKeyDown, this);

		m_Txt->Bind(ssEVT_AUTOCOMP_ENTRYSELECTED, &CInputWndBase::OnAutoComp_SelChanged, this);

		m_Txt->Bind(wxEVT_STC_MODIFIED, [&](wxStyledTextEvent& event)
		{
			if (event.GetLinesAdded() > 0) SwitchToMultiMode();
			
			event.Skip();
		});

		m_AutoComp = new AutoCompCtrl(m_Txt);
		m_AutoCompHelp = new AutoCompHelp(m_AutoComp);
		m_AutoComp->AttachHelpWindow(m_AutoCompHelp);
	}


	CInputWndBase::~CInputWndBase() = default;
	


	wxSize CInputWndBase::DoGetBestSize() const
	{
		wxClientDC dc(const_cast<CInputWndBase*> (this));

		wxCoord w = 0;
		wxCoord h = 0;
		dc.GetTextExtent(m_Txt->GetValue(), &w, &h);

		return wxSize(w, h);
	}


	void CInputWndBase::OnPaint(wxPaintEvent& event)
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



	void CInputWndBase::OnKeyUp(wxKeyEvent& evt)
	{
		int curPos = m_Txt->GetCurrentPos();
		int Style = m_Txt->GetStyleAt(curPos);

		if (Style == wxSTC_P_COMMENTBLOCK ||
			Style == wxSTC_P_COMMENTLINE ||
			Style == wxSTC_P_STRING)
		{
			evt.Skip();
			return;
		}

		int evtCode = evt.GetKeyCode();
		auto Char = evt.GetUnicodeKey();

		if ((m_Txt->AutoCompActive() && evtCode == WXK_BACK))
			ShowAutoComp();

		else if (Char == '.')
		{
			if (curPos <= 1)
				return;

			//Dont show the autocomp when user is typing a number such as 2.34
			double DummyValue;
			int Start = m_Txt->WordStartPosition(curPos - 1, true);
			int End = curPos - 1;
			wxString Word = m_Txt->GetTextRange(Start, End);
			bool Show = !Word.ToDouble(&DummyValue) && !Word.empty();
			if (Show)
				ShowAutoComp();
		}

		evt.Skip();
	}


	wxString CInputWndBase::ProcessCommand(const wxString& Command)
	{
		if (Command.empty())
			return wxEmptyString;

		int InputFlag = m_Mode == MODE::MULTI ? Py_file_input : Py_single_input;


		m_NExecCmds++;

		PyObject* EvalObj = nullptr;
		PyObject* CodeObject = nullptr;
		PyObject* DictObj = PyModule_GetDict(m_PyModule);

		std::wstring StdIOErr;

		std::string FileName = "Shell#" + std::to_string(m_NExecCmds);

		//string might contain UTF entries, so we encode it
		CodeObject = Py_CompileString(Command.mb_str(wxConvUTF8), FileName.c_str(), InputFlag);
		if (CodeObject)
		{
			EvalObj = PyEval_EvalCode(CodeObject, DictObj, DictObj);
			Py_DECREF(CodeObject);
			if (!EvalObj)
				PyErr_Print();
		}
		else
			PyErr_Print();

		m_stdOutErrCatcher.CaptureOutput(StdIOErr);

		Py_XDECREF(EvalObj);

		return StdIOErr;
	}


	void CInputWndBase::SwitchToMultiMode()
	{
		m_Mode = MODE::MULTI;
		m_StTxt->SetBackgroundColour(wxColour(0, 255, 0));
		m_StTxt->SetLabel("++");
		m_StTxt->Refresh();
	}

	void CInputWndBase::SwitchToSingleMode()
	{
		m_Mode = MODE::SINGLE;
		m_StTxt->SetBackgroundColour(m_StTxtDefBG);
		m_StTxt->SetLabel(">>");
		m_StTxt->Refresh();
	}


	void CInputWndBase::OnAutoComp_SelChanged(wxCommandEvent& event)
	{
		auto TxtRng = m_Txt->GetLineTextUntilCarret();
		wxString SelId = m_AutoComp->GetStringSelection();

		auto DocStr = script::GetDocString(TxtRng.ToStdWstring(), SelId.ToStdWstring(), m_PyModule);
		if (!DocStr.empty())
			m_AutoCompHelp->ShowHelp(DocStr);
	}


	void CInputWndBase::SwitchInputMode(wxCommandEvent& event)
	{
		if (m_Mode == MODE::SINGLE)
			SwitchToMultiMode();
		else
			SwitchToSingleMode();
	}


	void CInputWndBase::ShowAutoComp()
	{
		auto TextRange = m_Txt->GetLineTextUntilCarret();
		auto SymbolTbl = ExtractSymbolTable(TextRange.ToStdWstring(), m_PyModule);

		if (SymbolTbl.size() == 0)
			return;

		m_AutoComp->SetList(SymbolTbl);

		auto Word = m_AutoComp->GetCurrentWord();
		if (!Word.empty())
		{
			auto List = m_AutoComp->Filter(Word.ToStdWstring());
			m_AutoComp->Show(List);
		}
		else
			m_AutoComp->Show(SymbolTbl);
	}
}