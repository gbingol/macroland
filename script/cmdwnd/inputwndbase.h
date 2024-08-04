#pragma once

#include <variant>
#include <wx/wx.h>
#include <Python.h>

#include "../styledtxtctrl.h"
#include "../autocompletion.h"
#include "../scripting_funcs.h"

#include "../dllimpexp.h"



namespace script
{
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

		DLLSCRIPT void OnAutoComp_SelChanged(wxCommandEvent& event);

		DLLSCRIPT void SwitchInputMode(wxCommandEvent& event);

	protected:

		DLLSCRIPT void ShowAutoComp();

		DLLSCRIPT wxString ProcessCommand(const wxString& Command);

		DLLSCRIPT void SwitchToMultiMode();
		DLLSCRIPT void SwitchToSingleMode();

	protected:
		wxStaticText* m_StTxt;
		CScriptCtrlBase* m_Txt;

		/*
			If false, evaluates multiple lines when shift+enter pressed
			True: evaluates single line commands when enter pressed
		*/
		MODE m_Mode = MODE::SINGLE;

		AutoCompCtrl* m_AutoComp{ nullptr };

	private:
		wxWindow* m_ParentWnd;

		/*
			Static text's default background color
			The color is changed to green when mode is MULTI
			and back to default color when mode is SINGLE
		*/
		wxColor m_StTxtDefBG;

		//Help window next to auto-complete window
		AutoCompHelp* m_AutoCompHelp{ nullptr };

		PyObject* m_PyModule = nullptr;
		CStdOutErrCatcher m_stdOutErrCatcher;

		//when there is an error instead of showing "<string>", showing the nth command 
		size_t m_NExecCmds = 0;
	};
}
