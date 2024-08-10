#pragma once

#include <variant>
#include <wx/wx.h>
#include <Python.h>

#include "styledtxtctrl.h"
#include "autocompletion.h"

#include "dllimpexp.h"




namespace script
{
	class CStdOutErrCatcher
	{
	public:
		CStdOutErrCatcher(PyObject* moduleObj = nullptr)
		{
			m_ModuleObj = moduleObj;
		}

		void SetModule(PyObject* moduleObj)
		{
			m_ModuleObj = moduleObj;
		}

		DLLSCRIPT bool StartCatching() const;

		//returns false if something goes wrong, resets the internal value property
		DLLSCRIPT bool CaptureOutput(std::wstring& output) const;

		//restores the previous IO state
		DLLSCRIPT bool RestorePreviousIO() const;

	private:
		PyObject* m_ModuleObj = nullptr;
	};



	/******************************************************************* */


	class CInputWndBase : public wxControl
	{
	protected:
		enum class MODE { SINGLE, MULTI };

	public:
		DLLSCRIPT CInputWndBase(wxWindow* parent, PyObject* Module);

		DLLSCRIPT ~CInputWndBase();

		DLLSCRIPT wxSize DoGetBestSize() const;


		DLLSCRIPT auto GetScriptCtrl() const
		{
			return m_Txt;
		}

	protected:
		DLLSCRIPT void OnPaint(wxPaintEvent& event);

		virtual DLLSCRIPT void OnKeyDown(wxKeyEvent& event) = 0;
		virtual DLLSCRIPT void OnKeyUp(wxKeyEvent& evt);
		void OnChar(wxKeyEvent& event);


		DLLSCRIPT void SwitchInputMode(wxCommandEvent& event);

	protected:

		void ShowAutoComp();

		wxString ProcessCommand(const wxString& Command);

		void SwitchToMultiMode();
		void SwitchToSingleMode();

	protected:
		wxStaticText* m_StTxt;
		CScriptCtrlBase* m_Txt;

		/*
			If false, evaluates multiple lines when shift+enter pressed
			True: evaluates single line commands when enter pressed
		*/
		MODE m_Mode = MODE::SINGLE;

		AutoCompCtrl* m_AutoComp{ nullptr };
		frmParamsDocStr *m_ParamsDoc{nullptr};

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
}
