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




	/****************************************************** */

	class CStdOutErrCatcher
	{
	public:
		CStdOutErrCatcher(PyObject* Obj = nullptr):m_ModuleObj{Obj} { }

		bool StartCatching() const;

		//returns false if something goes wrong, resets the internal value property
		bool CaptureOutput(std::wstring& output) const;

	private:
		PyObject* m_ModuleObj = nullptr;
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

		virtual void OnKeyDown(wxKeyEvent& event) = 0;
		virtual void OnKeyUp(wxKeyEvent& evt);
		void OnChar(wxKeyEvent& event);


		void SwitchInputMode(wxCommandEvent& event);

	protected:

		void ShowAutoComp();

		wxString ProcessCommand(const wxString& Command);

		void SwitchToMultiMode();
		void SwitchToSingleMode();

	protected:
		wxStaticText* m_StTxt;
		script::CStyledTextCtrl* m_Txt;

		/*
			If false, evaluates multiple lines when shift+enter pressed
			True: evaluates single line commands when enter pressed
		*/
		MODE m_Mode = MODE::SINGLE;

		script::AutoCompCtrl* m_AutoComp{ nullptr };
		script::frmParamsDocStr *m_ParamsDoc{nullptr};

	private:
		wxWindow* m_ParentWnd;

		/*
			Static text's default background color
			The color is changed to green when mode is MULTI
			and back to default color when mode is SINGLE
		*/
		wxColor m_StTxtDefBG;


		PyObject* m_PyModule = nullptr;
		CStdOutErrCatcher m_stdOutErrCatcher;

		//when there is an error instead of showing "<string>", showing the nth command 
		size_t m_NExecCmds = 0;

		/*
			Different keyboards have different layouts and to detect the current character
			we need OnChar event.
			However, we might need to process this on OnKeyDown or OnKeyUp events. That's why
			we keep track of the current char at OnCharEvent
		*/
		wchar_t m_Char{};
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