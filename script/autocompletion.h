#pragma once

#include <wx/wx.h>
#include <wx/caret.h>
#include <wx/stc/stc.h> 
#include <wx/html/htmlwin.h>
#include <wx/listctrl.h>
#include <wx/popupwin.h>
#include <wx/minifram.h>

#include "dllimpexp.h"




DLLSCRIPT wxDECLARE_EVENT(ssEVT_AUTOCOMP_ENTRYSELECTED, wxCommandEvent);
DLLSCRIPT wxDECLARE_EVENT(ssEVT_AUTOCOMP_CANCELLED, wxCommandEvent);



namespace script
{

	class AutoCompCtrl :public wxMiniFrame
	{
	public:
		DLLSCRIPT AutoCompCtrl(
			wxStyledTextCtrl* stc, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200, 200));

		DLLSCRIPT ~AutoCompCtrl();


		//pair: first is key, second is the data type
		DLLSCRIPT void Show(const std::list<std::wstring>& List);

		DLLSCRIPT void SetList(const std::list<std::wstring>& List)
		{
			m_CurList = List;
		}

		DLLSCRIPT void Hide();

		DLLSCRIPT wxString GetStringSelection() const;

		//Give the focus to the control
		DLLSCRIPT void SetFocus(int Selection = -1);

		DLLSCRIPT wxString GetCurrentWord() const;

		DLLSCRIPT std::list<std::wstring> Filter(const std::wstring& str) const;

		//Attach a help window to set its position automatically when showing the AutoCompletion
		DLLSCRIPT void AttachHelpWindow(wxWindow* const HelpWindow);


		//Is there an auto-completion list visible
		DLLSCRIPT bool Active() const
		{
			return IsShown();
		}


		DLLSCRIPT bool IsActive() const
		{
			return IsShown();
		}

	protected:

		DLLSCRIPT void OnKeyDown(wxKeyEvent& evt);
		DLLSCRIPT void OnKeyUp(wxKeyEvent& evt);

		DLLSCRIPT void OnParentWindow_KeyDown(wxKeyEvent& event);
		DLLSCRIPT void OnParentWindow_KeyUp(wxKeyEvent& event);

		DLLSCRIPT void AutoComp_EntrySelected(wxListEvent& event);

	private:
		wxPoint ComputeShowPositon();

		//std::string is the Key, int is type of Key
		void PopulateControl(const std::list<std::wstring>& List);

	private:

		std::list<std::wstring> m_CurList; //the list that is being shown 
 
		wxStyledTextCtrl* m_STC = nullptr;
		wxListView* m_List{ nullptr };

		wxTopLevelWindow* m_PrntWnd{ nullptr };

		//The help window that will be shown when  selections are made at autocompletion
		wxWindow* m_HelpWnd = nullptr;
	};





	/******************************************************************/
	
	class AutoCompHelp : public wxPopupWindow
	{

	public:

		DLLSCRIPT AutoCompHelp(AutoCompCtrl* AutoCompCtrl);

		DLLSCRIPT ~AutoCompHelp() = default;

		DLLSCRIPT void ShowHelp(const wxString& HelpSource);

		DLLSCRIPT wxPoint GetComputedPos() const;

	protected:
		DLLSCRIPT void OnIdle(wxIdleEvent& event);

	private:

		//The window where we show the HTML text
		wxHtmlWindow* m_HTMLHelp;

		//This frame is owned by a control which shows it
		AutoCompCtrl* m_AutoComp{ nullptr };
	};
}