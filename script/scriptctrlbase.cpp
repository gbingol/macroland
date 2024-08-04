#include "scriptctrlbase.h"

#include <chrono>
#include <random>

#include <wx/caret.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/file.h>

#include "popuphtml.h"
#include "autocompletion.h"

#include "scripting_funcs.h"


namespace script
{

	CScriptCtrlBase::CScriptCtrlBase(wxWindow* parent, PyObject* Module) :
		CStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE, "")
	{
		m_PythonModule = Module;

		SetMouseDwellTime(500);//milliseconds
		m_frmInfo = new CPopupHTML(this, wxDefaultPosition, FromDIP(wxSize(400, 250)));
		m_frmInfo->Dismiss();

		Bind(wxEVT_KEY_DOWN, &CScriptCtrlBase::OnKeyUp, this);

		Bind(wxEVT_STC_CHARADDED, &CScriptCtrlBase::OnCharAdded, this);
		Bind(wxEVT_STC_DWELLSTART, &CScriptCtrlBase::OnDwellStart, this);
		Bind(wxEVT_STC_DWELLEND, &CScriptCtrlBase::OnDwellEnd, this);
		Bind(wxEVT_STC_MODIFIED, &CScriptCtrlBase::OnModified, this);

		m_AutoComp = new AutoCompCtrl(this);
		m_AutoCompHelp = new AutoCompHelp(m_AutoComp);

		Bind(ssEVT_AUTOCOMP_ENTRYSELECTED, &CScriptCtrlBase::OnAutoComp_EntrySelected, this);
	}


	CScriptCtrlBase::~CScriptCtrlBase()
	{
		Unbind(ssEVT_AUTOCOMP_ENTRYSELECTED, &CScriptCtrlBase::OnAutoComp_EntrySelected, this);
		if (m_frmInfo)
			delete m_frmInfo;
	}


	void CScriptCtrlBase::OnKeyUp(wxKeyEvent& evt)
	{
		int KeyCode = evt.GetKeyCode();

		if (m_AutoComp->Active() && KeyCode == WXK_BACK) 
		{
			wxString TextRange = GetLineTextUntilCarret();
			if (TextRange.empty()) 
			{
				m_AutoComp->Hide();
				return;
			}
		}

		else if (KeyCode == WXK_BACK ||
			KeyCode == WXK_EXECUTE ||
			KeyCode == WXK_DELETE ||
			m_CompileErr.LineNo >= 0) {
			CallAfter([this]
				{
					CompileAndShowErrors();
				});
		}

		evt.Skip();
	}


	void CScriptCtrlBase::OnCharAdded(wxStyledTextEvent& event)
	{
		int evtKey = event.GetKey();
		int Pos = GetCurrentPos();

		if (GetStyleAt(Pos) == wxSTC_P_COMMENTBLOCK ||
			GetStyleAt(Pos) == wxSTC_P_COMMENTLINE) 
		{
			event.Skip();
			return;
		}


		if (evtKey == '.' || m_AutoComp->Active()) 
		{
			auto TextRange = GetLineTextUntilCarret();
			if (TextRange.empty())
				return;

			auto IdArray = GetIdArray(TextRange.ToStdWstring());
			if (IdArray.empty())
				return;
		}

		else if (evtKey == ' ' && m_AutoComp->Active())
			m_AutoComp->Hide();

		else if (m_CompileErr.LineNo >= 0) {
			CompileAndShowErrors();
		}

		event.Skip();
	}



	void CScriptCtrlBase::OnDwellStart(wxStyledTextEvent& event)
	{
		wxPoint ScreenPos = wxGetMousePosition();
		wxPoint clientPos = ScreenToClient(ScreenPos);

		if (clientPos.x <= 0 || clientPos.y <= 0) {
			event.Skip();
			return;
		}

		long Pos = PositionFromPoint(clientPos);

		wxString LineText = GetLine(clientPos);
		if (LineText.empty()) {
			event.Skip();
			return;
		}


		int IndicatorVal = IndicatorValueAt(INDICATOR, Pos);
		if (IndicatorVal == INDICATOR_STYLE)
			PopupCompileErrorMessage(ScreenPos);
		else
			PopupDocString(ScreenPos);

		event.Skip();
	}


	void CScriptCtrlBase::OnDwellEnd(wxStyledTextEvent& event)
	{
		if (m_frmInfo) 
			m_frmInfo->Dismiss();

		event.Skip();
	}


	void CScriptCtrlBase::OnModified(wxStyledTextEvent& event)
	{	
		//if lines are added it is positive, if deleted it is negative
		int LinesAdded = event.GetLinesAdded();
		
		if (LinesAdded > 0)
		{
			int cur_pos = GetCurrentPos();
			int cur_line = LineFromPosition(cur_pos);

			if (!GetLineText(cur_line).empty())
				CompileAndShowErrors();
		}
		else if (LinesAdded < 0)
			CompileAndShowErrors();

		event.Skip();
	}



	void CScriptCtrlBase::OnAutoComp_EntrySelected(wxCommandEvent& event)
	{
		auto TxtRng = GetLineTextUntilCarret();

		wxString SelId = m_AutoComp->GetStringSelection();

		auto DocStr = GetDocString(TxtRng.ToStdWstring(), SelId.ToStdWstring(), m_PythonModule);
		if (DocStr.empty() == false)
			m_AutoCompHelp->ShowHelp(DocStr);
	}


	std::string CScriptCtrlBase::CreateRandomModuleName(size_t N) const
	{
		std::vector<int> Vec(N);

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<int> dist(1, 26);
		std::transform(Vec.begin(), Vec.end(), Vec.begin(), [&](int val) {return dist(generator); });

		std::stringstream ss;

		for (size_t i = 0; i < N; ++i)
			ss << char(('A' - 1) + Vec[i]);

		return ss.str();
	}



	void CScriptCtrlBase::HideAutoComplete() {
		m_AutoComp->Hide();
	}


	CScriptCtrlBase::CompileError CScriptCtrlBase::compile() const
	{
		CScriptCtrlBase::CompileError CompError;

		//Get the GIL
		auto gstate = PyGILState_Ensure();

		auto RandomStr = CreateRandomModuleName();
		PyObject* Module = PyModule_New(RandomStr.c_str());

		if (Module == nullptr) 
		{
			PyGILState_Release(gstate);
			return CompError;
		}

		auto Dict = PyModule_GetDict(Module);
		if (Dict == nullptr)
		{
			Py_DECREF(Module);
			PyGILState_Release(gstate);

			return CompError;
		}

		wxString ScriptTxt = GetText();
		if (ScriptTxt.empty())
		{
			PyGILState_Release(gstate);
			return CompError;
		}

		PyObject* ScriptObj = PyUnicode_FromString(GetText().mb_str(wxConvUTF8));
		if (!ScriptObj)
		{
			PyGILState_Release(gstate);
			return CompError;
		}

		PyDict_SetItemString(Dict, "_SYSTEM_scripttext", ScriptObj);
		PyDict_SetItemString(Dict, "_SYSTEM_LineNo", Py_BuildValue(""));
		PyDict_SetItemString(Dict, "_SYSTEM_Text", Py_BuildValue(""));
		PyDict_SetItemString(Dict, "_SYSTEM_Msg", Py_BuildValue(""));

		wxString PythonCmd = "try: \n";
		PythonCmd << "    _SYSTEM_LineNo=-1 \n";
		PythonCmd << "    _SYSTEM_Text=\"\" \n";
		PythonCmd << "    _SYSTEM_Msg=\"\" \n";
		PythonCmd << "    compile(_SYSTEM_scripttext, \"\", mode=\"exec\") \n";
		PythonCmd << "except Exception as e: \n";
		PythonCmd << "    _SYSTEM_LineNo=e.lineno \n";
		PythonCmd << "    _SYSTEM_Text=e.text \n";
		PythonCmd << "    _SYSTEM_Msg=e.msg";


		auto ResultObj = PyRun_String(PythonCmd.mb_str(wxConvUTF8), Py_file_input, Dict, Dict);
		if (ResultObj) 
		{
			PyObject* LineObj = PyDict_GetItemString(Dict, "_SYSTEM_LineNo");
			PyObject* TextObj = PyDict_GetItemString(Dict, "_SYSTEM_Text");
			PyObject* MsgObj = PyDict_GetItemString(Dict, "_SYSTEM_Msg");

			CompError.LineNo = PyLong_AsLong(LineObj);
			CompError.Text = PyUnicode_AsUTF8(TextObj);
			CompError.Msg = PyUnicode_AsUTF8(MsgObj);
		}

		Py_XDECREF(ScriptObj);
		Py_XDECREF(ResultObj);
		Py_XDECREF(Dict);
		Py_XDECREF(Module);

		PyGILState_Release(gstate);

		return CompError;
	}


	void CScriptCtrlBase::CompileAndShowErrors()
	{
		//clear the last error
		if (m_CompileErr.LineNo >= 0)
			ClearCompileErrors();

		m_CompileErr = compile();

		if (m_CompileErr.LineNo < 0 ||
			m_CompileErr.Text == wxEmptyString ||
			m_CompileErr.Msg == wxEmptyString)
			return;

		int IndicLineNo = m_CompileErr.LineNo - 1;

		int IndicStartPos = PositionFromLine(IndicLineNo);
		int PosEnd = GetLineEndPosition(IndicLineNo);

		int IndicLen = PosEnd - IndicStartPos;


		SetIndicatorCurrent(INDICATOR);
		IndicatorSetStyle(INDICATOR, INDICATOR_STYLE);
		IndicatorSetForeground(INDICATOR, wxColor(255, 0, 0));

		IndicatorFillRange(IndicStartPos, IndicLen);
	}


	void CScriptCtrlBase::ClearCompileErrors()
	{
		int IndicLineNo = m_CompileErr.LineNo - 1;

		int StartPos = PositionFromLine(IndicLineNo);
		int PosEnd = GetLineEndPosition(IndicLineNo);

		SetIndicatorCurrent(INDICATOR);
		IndicatorClearRange(StartPos, PosEnd - StartPos);

		m_CompileErr.LineNo = -1;
	}


	void CScriptCtrlBase::PopupCompileErrorMessage(const wxPoint& ScreenCoord)
	{		
		wxString ErrMsg = "<HTML><BODY>";
		ErrMsg << "<p><font color=red>" << m_CompileErr.Msg << "</font></p>";
		ErrMsg << "<p>" << m_CompileErr.Text << "</p>";
		ErrMsg << "</BODY></HTML>";

		auto Pos = ScreenCoord;
		Pos.y += TextHeight(0);

		m_frmInfo->SetPosition(Pos);
		m_frmInfo->SetSize(wxSize(300, 150));
		m_frmInfo->SetHTMLPage(ErrMsg);
		m_frmInfo->Popup();
	}


	void CScriptCtrlBase::PopupDocString(const wxPoint& ScreenCoord)
	{
		wxPoint clientPos = ScreenToClient(ScreenCoord);
		wxString LineText = GetLine(clientPos);
		if (LineText.empty())
			return;

		wxString ID = GetWord(clientPos);

		//no point to show any info on the following
		if (ID.empty() || ID == "None")
			return;

		auto DocStr = GetDocString(LineText.ToStdWstring(), ID.ToStdWstring(), m_PythonModule);
		if (DocStr.empty()) 
		{
			//Get the GIL
			auto gstate = PyGILState_Ensure();
			
			if (auto BuiltIns = PyImport_ImportModule("builtins"))
			{
				DocStr = GetDocString(LineText.ToStdWstring(), ID.ToStdWstring(), BuiltIns);
				Py_XDECREF(BuiltIns);
			}

			PyGILState_Release(gstate);
		}

		//we exhausted our options of search for docstring
		if (DocStr.empty()) 
			return;

		auto Pos = ScreenCoord;
		Pos.y += TextHeight(0);

		m_frmInfo->SetPosition(Pos);
		m_frmInfo->SetSize(wxSize(400, 250));
		m_frmInfo->SetHTMLPage(DocStr);
		m_frmInfo->Popup();
	}

}