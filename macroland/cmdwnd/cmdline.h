#pragma once

#include <filesystem>
#include <vector>
#include <list>
#include <variant>
#include <optional>

#include <wx/wx.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/minifram.h>

#include <script/styledtxtctrl.h>
#include <script/autocompletion.h>
#include <script/styledtxtctrl.h>
#include <script/inputwndbase.h>


namespace scripting::cmdedit
{
	class CCmdLine;

	wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_RETURN, wxCommandEvent);

	class COutputWnd : public script::CStyledTextCtrl
	{
	public:
		COutputWnd(
			wxWindow* parent,
			wxWindowID id = -1,
			script::CStyledTextCtrl* InputWnd = nullptr);

		~COutputWnd() = default;

		wxSize DoGetBestSize() const;

		void AppendOutput(const wxString& txt, bool PrependLine = true);

	protected:
		void OnModified(wxStyledTextEvent& event);

		void OnSetFocus(wxFocusEvent& event);
		void OnKillFocus(wxFocusEvent& event);

		void OnRightUp(wxMouseEvent& event);

		void OnPopMenu(wxCommandEvent& event);

	private:
		wxFont m_Font;
		wxWindow* m_PrntWnd;
		script::CStyledTextCtrl* m_InputWnd;

		const int ID_SAVE{ wxNewId() };
		const int ID_COPY{ wxNewId() };
		const int ID_DELALL{ wxNewId() };
		const int ID_SHOWLINENO{ wxNewId() };

		bool m_ShowLineNo{ true };
	};


	/****************    CInputWnd  ************************/

	class CInputWnd : public script::CInputWndBase
	{
	public:
		using COMMAND = std::variant<wxString, std::list<wxString>>;

	public:
		CInputWnd(CCmdLine* parent, PyObject* Module);

		~CInputWnd();

		auto GetContextMenu() const
		{
			return m_ContextMenu.get();
		}

		auto GetCommandHist() const
		{
			return m_CmdHist;
		}

		void AppendText(const wxString& txt)
		{
			m_Txt->AppendText(txt);
		}

		auto GetScriptCtrl() const
		{
			return m_Txt;
		}

	protected:
		void OnKeyDown(wxKeyEvent& evt) override;
		void OnReturn(wxCommandEvent& evt);

	private:
		bool OpenHistoryFile(std::string* Msg = nullptr);
		bool CloseHistoryFile();


	private:
		CCmdLine* m_ParentWnd;

		
		const int ID_BROWSEPATH{ wxNewId() };
		//single or multiline
		const int ID_INPUTMODE{ wxNewId() };
		//should we switch to single or multiline automatically
		const int ID_AUTOSWITCHINPUTMODE{ wxNewId() };


		//Position in the history list: 0 as current, 1 as previous 2,3, ....
		int m_HistPos;

		std::vector<COMMAND> m_CmdHist{};

		PyObject* m_PyModule = nullptr;

		std::unique_ptr<wxMenu> m_ContextMenu;
	};





	/**************************   CCmdLine  *************************************/

	class CCmdLine : public wxControl
	{
	public:
		CCmdLine(wxWindow* parent, PyObject* Module);

		~CCmdLine() = default;

		wxSize DoGetBestSize() const;

		auto GetInputWnd() const {
			return m_TxtInput;
		}

		auto GetOutputWnd() const {
			return m_TxtOutput;
		}

		auto GetCommandHist() const
		{
			return m_TxtInput->GetCommandHist();
		}

	protected:
		
		void OnPaint(wxPaintEvent& event);


	private:
		//Owning frame
		wxWindow* m_PrntWnd = nullptr;

		CInputWnd* m_TxtInput;
		COutputWnd* m_TxtOutput;
	};

}