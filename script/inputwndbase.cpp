#include "inputwndbase.h"


#include "scripting_funcs.h"


namespace script
{
	bool CStdOutErrCatcher::StartCatching() const
	{
		/*
		python code to redirect stdouts / stderr
		From: https://stackoverflow.com/questions/4307187/how-to-catch-python-stdout-in-c-code
		*/
		const std::string stdOutErr =
			"import sys\n\
class StdOutput:\n\
	def __init__(self):\n\
		self.value = ''\n\
		self.stdout=sys.stdout\n\
		self.stderr=sys.stderr\n\
	def write(self, txt):\n\
		self.value += txt\n\
	def restore(self):\n\
		sys.stdout=self.stdout\n\
		sys.stderr=self.stderr\n\
CATCHSTDOUTPUT = StdOutput()\n\
sys.stdout = CATCHSTDOUTPUT\n\
sys.stderr = CATCHSTDOUTPUT\n\
			";

		if (m_ModuleObj == nullptr)
			return false;

		auto gstate = GILStateEnsure();

		if (auto py_dict = PyModule_GetDict(m_ModuleObj))
		{
			if (auto ResultObj = PyRun_String(stdOutErr.c_str(), Py_file_input, py_dict, py_dict))
				return true;
		}

		return false;
	}



	bool CStdOutErrCatcher::CaptureOutput(std::wstring& output) const
	{
		auto gstate = GILStateEnsure();

		PyObject* py_dict = PyModule_GetDict(m_ModuleObj);
		if (!py_dict)
			return false;

		PyObject* catcher = PyDict_GetItemString(py_dict, "CATCHSTDOUTPUT");
		if (!catcher)
			return false;

		PyObject* OutputObj = PyObject_GetAttrString(catcher, "value");
		if (!OutputObj)
			return false;

		output = PyUnicode_AsWideCharString(OutputObj, nullptr);
		PyObject_SetAttrString(catcher, "value", Py_BuildValue("s", ""));

		return true;
	}



	bool CStdOutErrCatcher::RestorePreviousIO() const
	{
		auto gstate = GILStateEnsure();

		PyObject* py_dict = PyModule_GetDict(m_ModuleObj);
		if (!py_dict)
			return false;

		PyObject* catcher = PyDict_GetItemString(py_dict, "CATCHSTDOUTPUT");
		if (!catcher)
			return false;

		auto CallResult = PyObject_CallMethodNoArgs(catcher, Py_BuildValue("s", "restore"));

		return true;
	}

	
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
		m_Txt->Bind(wxEVT_CHAR, &CInputWndBase::OnChar, this);

		m_AutoComp = new AutoCompCtrl(m_Txt);
		m_ParamsDoc = new FuncParamsDocStr(m_Txt);

		m_Txt->Bind(wxEVT_STC_MODIFIED, [&](wxStyledTextEvent& event)
		{
			if (event.GetLinesAdded() > 0) SwitchToMultiMode();
			
			event.Skip();
		});
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
		
		//TODO: This does not make sense (AutoCompActive is wxStyled's control)
		if ((m_Txt->AutoCompActive() && evtCode == WXK_BACK))
			ShowAutoComp();

		else if (Char == '.')
		{
			if (curPos <= 1)
				return;

			//Dont show the autocomp when user is typing a number such as 2.34
			double DummyValue;
			wxString Word = m_Txt->GetPreviousWord(curPos - 1);
			bool Show = !Word.ToDouble(&DummyValue) && !Word.empty();
			if (Show)
				ShowAutoComp();
		}

		evt.Skip();
	}



	void CInputWndBase::OnChar(wxKeyEvent &event)
	{
		int evtCode = event.GetKeyCode();
		auto Char = event.GetUnicodeKey();

		if(Char == '(')
		{
			int curPos = m_Txt->GetCurrentPos();
			if(curPos == 0)
			{
				event.Skip();
				return;
			}

			wxString Word = m_Txt->GetPreviousWord(curPos);
			auto Params = GetFuncParamsDocStr(Word.ToStdString(wxConvUTF8), m_PyModule);
			if(!Params.Doc.empty() || !Params.Params.empty())
				m_ParamsDoc->Show(std::make_pair(Params.Params, Params.Doc));
		}

		else if(Char == ')')
		{
			m_ParamsDoc->Hide();
		}

		event.Skip();
	}


	wxString CInputWndBase::ProcessCommand(const wxString& Command)
	{
		if (Command.empty())
			return wxEmptyString;

		int Flag = m_Mode == MODE::MULTI ? Py_file_input : Py_single_input;

		m_NExecCmds++;

		//ensure we have the GIL
		GILStateEnsure();

		PyObject *EvalObj{nullptr}, *CodeObj{nullptr};
		PyObject* DictObj = PyModule_GetDict(m_PyModule);

		//File name
		auto FName = ("Shell#" + std::to_string(m_NExecCmds)).c_str();

		//string might contain UTF entries, so we encode it
		CodeObj = Py_CompileString(Command.mb_str(wxConvUTF8), FName, Flag);
		if (CodeObj)
		{
			EvalObj = PyEval_EvalCode(CodeObj, DictObj, DictObj);
			Py_DECREF(CodeObj);
			if (!EvalObj)
				PyErr_Print();
		}
		else
			PyErr_Print();

		std::wstring StdIOErr;
		m_stdOutErrCatcher.CaptureOutput(StdIOErr);

		Py_XDECREF(EvalObj);

		return StdIOErr;
	}


	void CInputWndBase::SwitchToMultiMode()
	{
		m_Mode = MODE::MULTI;
		m_StTxt->SetBackgroundColour(wxColour(0, 255, 0));
		m_StTxt->SetLabel("++");
	}

	void CInputWndBase::SwitchToSingleMode()
	{
		m_Mode = MODE::SINGLE;
		m_StTxt->SetBackgroundColour(m_StTxtDefBG);
		m_StTxt->SetLabel(">>");
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
		int pos = m_Txt->GetCurrentPos();
		if(pos == 0)
			return;

		auto word = m_Txt->GetPreviousWord(pos);
		if(!word.empty())
		{
			auto SymbolTbl = GetObjectElements(word.ToStdString(wxConvUTF8), m_PyModule);

			if (SymbolTbl.size() > 0)
				m_AutoComp->Show(SymbolTbl);
		}
	}
}