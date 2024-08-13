#pragma once

#include <filesystem>
#include <vector>
#include <list>
#include <variant>
#include <optional>

#include <Python.h>

#include <wx/wx.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/minifram.h>

#include <script/styledtxtctrl.h>
#include <script/autocompletion.h>
#include <script/styledtxtctrl.h>


namespace cmdedit
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




	/******************************************************************* */


	class CInputWndBase : public wxControl
	{
	protected:
		enum class MODE { SINGLE, MULTI };

	public:
		CInputWndBase(wxWindow* parent, PyObject* Module);

		~CInputWndBase();

		wxSize DoGetBestSize() const;


		auto GetScriptCtrl() const
		{
			return m_Txt;
		}

	protected:
		void OnPaint(wxPaintEvent& event);
		void SwitchInputMode(wxCommandEvent& event);

	protected:
		void SwitchToMultiMode();
		void SwitchToSingleMode();

	protected:
		script::CStyledTextCtrl* m_Txt;

		/*
			If false, evaluates multiple lines when shift+enter pressed
			True: evaluates single line commands when enter pressed
		*/
		MODE m_Mode = MODE::SINGLE;

	private:
		wxWindow* m_ParentWnd;

		PyObject* m_PyModule = nullptr;
	};





	/****************    CInputWnd  ************************/

	class CInputWnd : public CInputWndBase
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
		void OnChar(wxKeyEvent& event);
		void OnKeyDown(wxKeyEvent& evt);
		void OnKeyUp(wxKeyEvent& evt);
		void OnReturn(wxCommandEvent& evt);

	private:
		void ShowAutoComp();
		wxString ProcessCommand(const char* Command); //UTF8

		//Open and then read contents of history file
		bool OpenReadHist();

		//write to and then close history file
		bool WriteCloseHist();


	private:
		CCmdLine* m_ParentWnd;

		script::AutoCompCtrl* m_AutoComp{ nullptr };
		script::frmParamsDocStr *m_ParamsDoc{nullptr};

		
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

		/*
			Different keyboards have different layouts and to detect the current character
			we need OnChar event.
			However, we might need to process this on OnKeyDown or OnKeyUp events. That's why
			we keep track of the current char at OnCharEvent
		*/
		wchar_t m_Char{};
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