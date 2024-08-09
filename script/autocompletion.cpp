#include "autocompletion.h"

#include <sstream>


#define IF_SKIP_RET(cond) \
	if(cond) {event.Skip(); return;}


namespace script
{

	AutoCompCtrl::AutoCompCtrl(	wxStyledTextCtrl* stc, wxWindowID id, const wxPoint& pos, const wxSize& size) :
		wxMiniFrame(stc, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTAB_TRAVERSAL)
	{
		m_STC = stc;

		m_ListBox = new wxListBox(this, id, pos, size);
		
		m_ListBox->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnKeyDown, this);
		m_ListBox->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnKeyUp, this);
		m_ListBox->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent &evt)
		{
			InsertSelection();
			return; 
		});

		auto Szr = new wxBoxSizer(wxVERTICAL);
		Szr->Add(m_ListBox, 1, wxEXPAND, 5);
		SetSizerAndFit(Szr);
		Layout();

		m_STC->Bind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnParentWindow_KeyDown, this);
		m_STC->Bind(wxEVT_KEY_UP, &AutoCompCtrl::OnParentWindow_KeyUp, this);
	}


	AutoCompCtrl::~AutoCompCtrl()
	{
		Unbind(wxEVT_KEY_DOWN, &AutoCompCtrl::OnKeyDown, this);
		Unbind(wxEVT_KEY_UP, &AutoCompCtrl::OnKeyUp, this);

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

		if (evtCode == WXK_RETURN)
		{
			InsertSelection();
			return;
		}

		event.Skip();
	}


	void AutoCompCtrl::OnParentWindow_KeyDown(wxKeyEvent& event)
	{
		int evtCode = event.GetKeyCode();

		if(IsShown())
		{
			if (evtCode == WXK_ESCAPE)
				Hide();

			else if (evtCode == WXK_UP || evtCode == WXK_DOWN || evtCode == WXK_RETURN)
			{
				wxMiniFrame::SetFocus();
				if(m_ListBox->GetSelection() == -1)
					m_ListBox->SetSelection(0);
				
				m_ListBox->SetFocus();

				//dont let parents OnKeyDown to be called, as AutoComp has the focus
				return;
			}

			else if(evtCode == WXK_BACK)
			{
				int pos = m_STC->GetCurrentPos();
				auto str = m_STC->GetTextRange(pos-1, pos);
				if(str == ".")
					Hide();
			}
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

		for (size_t i = 0; i < m_ListBox->GetCount(); ++i)
		{
			auto s = m_ListBox->GetString(i);
			if(s.substr(0, word.length()) == word)
			{
				m_ListBox->SetSelection(i);
				break;
			}
		}
			
		event.Skip();
	}



	void AutoCompCtrl::Hide()
	{
		if (!IsShown())
			return;

		wxMiniFrame::Hide();
		m_STC->SetFocus();
	}


	void AutoCompCtrl::Show(const std::list<std::string>& List)
	{
		if (List.size() == 0) {
			Hide();
			return;
		}

		m_ListBox->Clear();

		for (const auto& elem : List)
			m_ListBox->Append(elem);

		wxString word = GetCurrentWord();
		if (!word.empty())
		{
			for (size_t i = 0; i < m_ListBox->GetCount(); ++i)
			{
				auto s = m_ListBox->GetString(i);
				if(s.substr(0, word.length()) == word)
				{
					m_ListBox->SetSelection(i);
					break;
				}
			}
		}

		SetPosition(ComputeShowPositon());
		wxMiniFrame::Show(true);
	}



	wxString AutoCompCtrl::GetCurrentWord() const
	{
		int StartPos = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		int EndPos = m_STC->WordEndPosition(StartPos, true);

		if (EndPos > StartPos)
			return m_STC->GetTextRange(StartPos, EndPos);

		return wxEmptyString;
	}


	void AutoCompCtrl::InsertSelection()
	{
		int posSt = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
		int posEnd = m_STC->WordEndPosition(posSt, true);
		m_STC->DeleteRange(posSt, posEnd - posSt);

		wxString SelTxt = m_ListBox->GetStringSelection();
		m_STC->WriteText(SelTxt);
		m_STC->SetCurrentPos(posSt + SelTxt.length());

		Hide();
		m_STC->SetFocus();
	}

	wxPoint AutoCompCtrl::ComputeShowPositon()
	{
		wxPoint TL;

		int curPos = m_STC->WordStartPosition(m_STC->GetCurrentPos(), true);
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



	/************************************************************************ */

	FuncParamsDocStr::FuncParamsDocStr(	wxStyledTextCtrl* stc, wxWindowID id, const wxPoint& pos, const wxSize& size) :
		wxMiniFrame(stc, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTAB_TRAVERSAL)
	{
		m_STC = stc;

		m_HTMLWnd = new wxHtmlWindow(this, id, pos, size);

		auto Szr = new wxBoxSizer(wxVERTICAL);
		Szr->Add(m_HTMLWnd, 1, wxEXPAND, 5);
		SetSizer(Szr);
		Layout();

		m_HTMLWnd->Bind(wxEVT_KEY_DOWN, &FuncParamsDocStr::OnKeyDown, this);
		m_STC->Bind(wxEVT_KEY_DOWN, &FuncParamsDocStr::OnParentWindow_KeyDown, this);
	}

	FuncParamsDocStr::~FuncParamsDocStr() = default;

	void FuncParamsDocStr::OnKeyDown(wxKeyEvent &evt)
	{
		int evtCode = evt.GetKeyCode();	 
		if (evtCode == WXK_ESCAPE && IsShown()) {
			Hide();
			return;
		}

		evt.Skip();
	}


	void FuncParamsDocStr::OnParentWindow_KeyDown(wxKeyEvent &event)
	{
		int evtCode = event.GetKeyCode();

		if(IsShown() && evtCode == WXK_ESCAPE)
			Hide();
		
		else if(evtCode == WXK_BACK)
		{
			int pos = m_STC->GetCurrentPos();
			auto str = m_STC->GetTextRange(pos-1, pos);
			if(str == "(")
				Hide();
		}

		event.Skip();
	}


	void FuncParamsDocStr::Show(const std::pair<wxString, wxString> text)
	{
		const auto [Params, Doc] = text;

		std::stringstream ss;
		ss << "<HTML><BODY>" << "\n";
		if(!Params.empty())
			ss << "<p>" << Params.ToStdString(wxConvUTF8) << "</p>" << "\n";
		
		if(!Doc.empty())
			ss << "<p>" << Doc.ToStdString(wxConvUTF8) << "</p"<<"\n";
		ss << "</BODY></HTML>" <<

		m_HTMLWnd->SetPage(wxString::FromUTF8(ss.str()));

		SetPosition(ComputeShowPositon());
		wxMiniFrame::Show(true);
	}

	

	void FuncParamsDocStr::Hide()
	{
		wxMiniFrame::Hide();
		m_STC->SetFocus();
	}



	wxPoint FuncParamsDocStr::ComputeShowPositon()
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


}