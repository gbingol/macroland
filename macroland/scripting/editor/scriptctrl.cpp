#include "scriptctrl.h"

#include <wx/file.h>
#include <wx/artprov.h>

#include "scriptnotebook.h" 

#include "../../mainfrm/frmmacroland.h"


wxDEFINE_EVENT(ssEVT_SCRIPTCTRL_CARETPOSCHANGED, wxCommandEvent);






namespace scripting::editor
{
	ScriptCtrl::ScriptCtrl(ScriptNtbk* parent) :
		script::CScriptCtrlBase(parent, nullptr)
	{
		m_ParentWindow = parent;
	
		m_LastSaved = wxDateTime::Now();

		Bind(wxEVT_KEY_UP, &ScriptCtrl::OnKeyUp, this);
		Bind(wxEVT_LEFT_UP, &ScriptCtrl::OnLeftUp, this);
		Bind(wxEVT_LEFT_DOWN, &ScriptCtrl::OnLeftDown, this);

		Bind(wxEVT_SET_FOCUS, &ScriptCtrl::OnFocus, this);
		Bind(wxEVT_KILL_FOCUS, &ScriptCtrl::OnFocusLost, this);

		Bind(ssEVT_SCRIPTCTRL_CARETPOSCHANGED, &ScriptCtrl::OnCaretPositionChanged, this);

		Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent& event)
		{
			if (event.ControlDown() && event.GetKeyCode() == 'S')
				Save();

			event.Skip();
		});
	}


	ScriptCtrl::~ScriptCtrl()
	{
		Py_XDECREF(m_PythonModule);
	}


	PyObject* ScriptCtrl::GetPythonModule()
	{
		if (!m_PythonModule)
		{
			auto RandomStr = CreateRandomModuleName();
			m_PythonModule = PyModule_New(RandomStr.c_str());
		}		

		return m_PythonModule;
	}


	void ScriptCtrl::OnKeyUp(wxKeyEvent& evt)
	{
		int KeyCode = evt.GetKeyCode();

		auto StatBar = ((frmMacroLand*)wxTheApp->GetTopWindow())->GetStatusBar();

		long col, line;
		PositionToXY(GetInsertionPoint(), &col, &line);

		if (KeyCode == WXK_INSERT || KeyCode == WXK_NUMPAD_INSERT)
			StatBar->SetStatusText(GetOvertype() ? "OVR" : "INS", STATBARFIELD);


		wxCommandEvent caretPosChanged(ssEVT_SCRIPTCTRL_CARETPOSCHANGED);
		wxPostEvent(this, caretPosChanged);

		evt.Skip();
	}


	void ScriptCtrl::OnLeftDown(wxMouseEvent& event)
	{
		wxCommandEvent caretPosChanged(ssEVT_SCRIPTCTRL_CARETPOSCHANGED);
		wxPostEvent(this, caretPosChanged);

		event.Skip();
	}


	void ScriptCtrl::OnLeftUp(wxMouseEvent& event)
	{
		int curPos = GetCurrentPos();

		int curPosStyle = GetStyleAt(curPos);
		if (curPosStyle != wxSTC_P_IDENTIFIER)
		{
			event.Skip();
			return;
		}

		int wordStart = WordStartPosition(curPos, true);
		int wordEnd = WordEndPosition(wordStart, true);
	
		//identifier
		wxString ID = GetRange(wordStart, wordEnd);

		wxString Msg;

		auto StartPos = FindAllText(ID, wxSTC_FIND_WHOLEWORD);
		if (StartPos.size() > 0)
		{
			//transform positions to line numbers
			std::ranges::transform(StartPos, StartPos.begin(), [this](int x) 
				{
					return LineFromPosition(x) + 1;
				});

			//keep unique line numbers
			std::ranges::sort(StartPos);
			const auto y = std::ranges::unique(StartPos);
			StartPos.erase(y.begin(), y.end());

			//append line numbers to msg
			std::ranges::for_each(StartPos, [&](int i) {Msg << i << ", "; });

			//remove ", "
			Msg.RemoveLast(2);
		}

		if (!Msg.empty())
		{
			auto StatBar = ((frmMacroLand*)wxTheApp->GetTopWindow())->GetStatusBar();
			StatBar->SetStatusText("lines: " + Msg, STATBARFIELD);
		}

		event.Skip();
	}


	void ScriptCtrl::OnFocus(wxFocusEvent& event)
	{
		Bind(wxEVT_STC_MODIFIED, &ScriptCtrl::OnModified, this);
		event.Skip();
	}



	void ScriptCtrl::OnFocusLost(wxFocusEvent& event)
	{
		Unbind(wxEVT_STC_MODIFIED, &ScriptCtrl::OnModified, this);
		event.Skip();
	}



	void ScriptCtrl::OnCaretPositionChanged(wxCommandEvent& event)
	{
		long curPos = GetCurrentPos();

		int cur_line = LineFromPosition(curPos);
		int col = GetColumn(curPos);

		wxString StatusBarMessage;
		StatusBarMessage << "Line: " << cur_line + 1 << ", Col: " << col + 1;

		auto StatBar = ((frmMacroLand*)wxTheApp->GetTopWindow())->GetStatusBar();
		StatBar->SetStatusText(StatusBarMessage, STATBARFIELD);
	}



	void ScriptCtrl::UpdateTabInfo()
	{
		std::wstring Title;
		std::filesystem::path Path;

		if (!GetPath().empty())
		{
			Title = GetPath().filename().wstring();
			Path = GetPath();
		}

		if (!Title.empty())
			SetTitle(IsModified() ? Title + L"*" : Title);

		size_t PageNum = m_ParentWindow->FindScript(this).value();
		m_ParentWindow->SetPageText(PageNum, GetTitle());
		m_ParentWindow->SetPageToolTip(PageNum, Path.wstring());
	}



	void ScriptCtrl::OnModified(wxStyledTextEvent& event)
	{
		UpdateTabInfo();
		event.Skip();
	}



	bool ScriptCtrl::Save(const std::filesystem::path& FullPath)
	{
		try
		{
			script::CStyledTextCtrl::SaveStyledText(FullPath);
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what());
		}
	
		UpdateTabInfo();
		SetLastSaved(wxDateTime::Now());

		return true;
	}




	bool ScriptCtrl::Save()
	{
		auto Path = GetPath();

		if (Path.empty())	
		{
			wxFileDialog SaveDialog(this, "Save Script", wxEmptyString, wxEmptyString,
				"Python Script (*.py)|*.py", wxFD_SAVE, wxDefaultPosition);

			if (SaveDialog.ShowModal() == wxID_OK)
				Path = SaveDialog.GetPath().ToStdWstring();
			else
				return false;
		}

		try
		{
			script::CStyledTextCtrl::SaveStyledText(Path);
			UpdateTabInfo();
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what());
		}

		return true;
	}
}