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

#include "styledtxtctrl.h"
#include "autocompletion.h"


namespace cmdedit
{
	class CCmdLine;

	wxDECLARE_EVENT(ssEVT_SCRIPTCTRL_RETURN, wxCommandEvent);

	class COutputWnd : public CStyledTextCtrl
	{
	public:
		COutputWnd(
			wxWindow* parent,
			wxWindowID id = -1,
			CStyledTextCtrl* InputWnd = nullptr);

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
		CStyledTextCtrl* m_InputWnd;

		const int ID_SAVE{ wxNewId() };
		const int ID_COPY{ wxNewId() };
		const int ID_DELALL{ wxNewId() };
		const int ID_SHOWLINENO{ wxNewId() };

		bool m_ShowLineNo{ true };
	};



	

	/****************    CInputWnd  ************************/

	class CInputWnd : public wxControl
	{
	public:
		using COMMAND = std::variant<wxString, std::list<wxString>>;

	public:
		CInputWnd(CCmdLine* parent, PyObject* Module);

		~CInputWnd();

		wxSize DoGetBestSize() const;


		auto GetScriptCtrl() const
		{
			return m_Txt;
		}


		auto GetCommandHist() const
		{
			return m_CmdHist;
		}

		void AppendText(const wxString& txt)
		{
			m_Txt->AppendText(txt);
		}

	protected:
		void OnPaint(wxPaintEvent& event);
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

		void SwitchToMultiMode();
		void SwitchToSingleMode();


	private:
		//Single or Multi-line mode
		enum class MODE { S, M };
		MODE m_Mode = MODE::S;

		CCmdLine* m_ParentWnd{nullptr};
		CStyledTextCtrl* m_Txt;
		
		wxStaticText* m_StTxt;
		wxColor m_StTxtDefBG;

		AutoCompCtrl* m_AutoComp{ nullptr };
		frmParamsDocStr *m_ParamsDoc{nullptr};


		//Position in the history list: 0 as current, 1 as previous 2,3, ....
		int m_HistPos;

		std::vector<COMMAND> m_CmdHist{};

		PyObject* m_PyModule{nullptr};

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