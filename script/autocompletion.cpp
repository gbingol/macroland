#include "autocompletion.h"


wxDEFINE_EVENT(ssEVT_AUTOCOMP_ENTRYSELECTED, wxCommandEvent);
wxDEFINE_EVENT(ssEVT_AUTOCOMP_CANCELLED, wxCommandEvent);


#define IF_SKIP_RET(cond) \
	if(cond) {event.Skip(); return;}


namespace script
{

	AutoCompCtrl::AutoCompCtrl(	wxStyledTextCtrl* stc, wxWindowID id, const wxPoint& pos, const wxSize& size) :
		wxMiniFrame(stc, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTAB_TRAVERSAL)
	{
		m_STC = stc;

		m_List = new wxListView(this, id, pos, size, wxLC_REPORT);
		
		m_List->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnKeyDown, this);
		m_List->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnKeyUp, this);
		m_List->Bind(wxEVT_LIST_ITEM_SELECTED, &AutoCompCtrl::AutoComp_EntrySelected, this);

		auto Szr = new wxBoxSizer(wxVERTICAL);
		Szr->Add(m_List, 1, wxEXPAND, 5);
		SetSizerAndFit(Szr);
		Layout();

		m_STC->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnParentWindow_KeyDown, this);
		m_STC->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnParentWindow_KeyUp, this);

		wxItemAttr ColumnHeader;
		ColumnHeader.SetFont(wxFontInfo(10).FaceName("Consolas").Italic());

		m_List->InsertColumn(0, "Suggestions");
		m_List->SetColumnWidth(0, GetRect().width);
		m_List->SetHeaderAttr(ColumnHeader);
	}


	AutoCompCtrl::~AutoCompCtrl()
	{
		Unbind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnKeyDown, this);
		Unbind(wxEVT_KEY_UP, &AutoCompCtrl::OnKeyUp, this);
		Unbind(wxEVT_LIST_ITEM_SELECTED, &AutoCompCtrl::AutoComp_EntrySelected, this);

		m_STC->Unbind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnParentWindow_KeyDown, this);
	}


	void AutoCompCtrl::OnKeyDown(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();
		auto Lst = { WXK_ESCAPE , WXK_LEFT ,WXK_RIGHT };
		 
		if (std::ranges::find(Lst, evtCode) != Lst.end())
		{
			Hide();
			return;
		}

		event.Skip();
	}


	void AutoCompCtrl::OnKeyUp(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();

		if (evtCode == WXK_UP || evtCode == WXK_DOWN)
		{
			wxPostEvent(m_STC, wxCommandEvent(ssEVT_AUTOCOMP_ENTRYSELECTED));
			return;
		}

		else if (evtCode == WXK_RETURN)
		{
			Hide();
			m_STC->SetFocus();

			int posSt = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
			int posEnd = m_STC->WordEndPosition(posSt, true);
			m_STC->DeleteRange(posSt, posEnd - posSt);

			wxString SelTxt = GetStringSelection();
			m_STC->WriteText(SelTxt);
			m_STC->SetCurrentPos(posSt + SelTxt.length());
			return;
		}

		event.Skip();
	}


	void AutoCompCtrl::OnParentWindow_KeyDown(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();

		if (evtCode == WXK_ESCAPE && IsShown())
			Hide();

		else if ((evtCode == WXK_UP || evtCode == WXK_DOWN) && IsShown())
		{
			SetFocus(0);

			//dont let parents OnKeyDown to be called, as AutoComp has the focus
			return;
		}

		event.Skip();
	}


	void AutoCompCtrl::OnParentWindow_KeyUp(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();
		auto UniCode = event.GetUnicodeKey();

		IF_SKIP_RET(!IsShown() || UniCode == '.');

		//if following are not skipped then AutoComp shows and disappears
		IF_SKIP_RET(evtCode == WXK_TAB || evtCode == WXK_SPACE || evtCode == WXK_CONTROL);

		wxString word = GetCurrentWord();
		if (word.empty())
		{
			if (IsShown()) Hide();

			event.Skip();
			return;
		}

		auto List = Filter(word.ToStdWstring());
		Show(List);
		
		event.Skip();
	}


	void AutoCompCtrl::AutoComp_EntrySelected(wxListEvent& event)
	{
		wxPostEvent(m_STC, wxCommandEvent(ssEVT_AUTOCOMP_ENTRYSELECTED));
	}


	void AutoCompCtrl::Hide()
	{
		if (!IsShown())
			return;

		wxMiniFrame::Hide();
		m_STC->SetFocus();

		if (m_HelpWnd)
			m_HelpWnd->Hide();

		wxPostEvent(this, wxCommandEvent(ssEVT_AUTOCOMP_CANCELLED));
	}


	void AutoCompCtrl::Show(const std::list<std::wstring>& List)
	{
		if (List.size() == 0) 
		{
			Hide();
			return;
		}

		m_List->DeleteAllItems();

		PopulateControl(List);

		SetPosition(ComputeShowPositon());

		wxMiniFrame::Show(true);
	}


	std::list<std::wstring> AutoCompCtrl::Filter(const std::wstring& str) const
	{
		std::list<std::wstring> Filtered;
		for (const auto& elem : m_CurList) 
		{
			if (elem.find(str) != std::wstring::npos)
				Filtered.push_back(elem);
		}

		return Filtered;
	}


	wxString AutoCompCtrl::GetCurrentWord() const
	{
		int StartPos = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		int EndPos = m_STC->WordEndPosition(StartPos, true);

		if (EndPos > StartPos)
			return m_STC->GetTextRange(StartPos, EndPos);

		return wxEmptyString;
	}


	wxString AutoCompCtrl::GetStringSelection() const
	{
		long item = m_List->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		return (item == -1) ? wxString() : m_List->GetItemText(item);
	}


	void AutoCompCtrl::SetFocus(int Selection)
	{
		if (Selection < 0) 	return;

		wxMiniFrame::SetFocus();
		m_List->SetFocus();
		m_List->Select(0);

		wxCommandEvent event;
		event.SetEventType(ssEVT_AUTOCOMP_ENTRYSELECTED);
		wxPostEvent(m_STC, event);
	}


	void AutoCompCtrl::AttachHelpWindow(wxWindow* const HelpWindow)
	{
		m_HelpWnd = HelpWindow;
	}


	wxPoint AutoCompCtrl::ComputeShowPositon()
	{
		wxPoint TL;

		int curPos = m_STC->GetCurrentPos();
		long col, line;
		m_STC->PositionToXY(curPos, &col, &line);

		wxPoint pos = m_STC->PointFromPosition(curPos);
		int TxtHeight = m_STC->TextHeight(line);

		TL = m_STC->ClientToScreen(pos);
		TL.y += TxtHeight;


		int Btm_Y = m_STC->GetScreenPosition().y + GetSize().GetHeight();
		int ScreenY = wxGetDisplaySize().GetHeight();

		/*
			works well when the parent is m_STC
			(however, note that at the bottom line AutoComp is only partially shown)
		*/
		if (Btm_Y > ScreenY)
			TL.y = TL.y - GetSize().y;

		return TL;
	}


	void AutoCompCtrl::PopulateControl(const std::list<std::wstring>& List)
	{
		for (long index = 0; const auto& elem : List)
			m_List->InsertItem(index++, elem);
	}



	/**************************   frmAutoCompHelp   *******************************************/


	AutoCompHelp::AutoCompHelp(AutoCompCtrl* AutoComp) :
		wxPopupWindow(AutoComp->GetParent())
	{
		m_AutoComp = AutoComp;

		SetSizeHints(wxDefaultSize, wxDefaultSize);
		SetSize(FromDIP(wxSize(300, 200)));

		auto szrMain = new wxBoxSizer(wxVERTICAL);

		m_HTMLHelp = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO);
		szrMain->Add(m_HTMLHelp, 1, wxEXPAND, 5);

		SetSizer(szrMain); //do not use SetSizerandFit
		Layout();

		Bind(wxEVT_IDLE, &AutoCompHelp::OnIdle, this);
	}


	void AutoCompHelp::ShowHelp(const wxString& HelpSource)
	{
		wxString HTMLText = HelpSource;

		if (!HTMLText.empty())
		{
			m_HTMLHelp->SetPage(HTMLText);

			if (!IsShown()) 
			{
				auto Pos = GetComputedPos();
				SetPosition(Pos);
				Show();
			}

			m_AutoComp->SetFocus();
		}
		else 
			if (IsShown()) Hide();
	}


	wxPoint AutoCompHelp::GetComputedPos() const
	{
		int ScreenHeight = wxGetDisplaySize().y;
		int Height = GetSize().GetHeight();
		
		wxSize szAutoComp = m_AutoComp->GetSize();
		wxPoint TL = m_AutoComp->GetScreenPosition();

		if ((TL.y + Height) > ScreenHeight)
			TL.y -= std::abs(Height - szAutoComp.y);

		return wxPoint(TL.x + szAutoComp.x, TL.y);
	}


	void AutoCompHelp::OnIdle(wxIdleEvent& event)
	{
		if(!m_AutoComp->IsActive())
			Hide();
		
		event.Skip();
	}
}