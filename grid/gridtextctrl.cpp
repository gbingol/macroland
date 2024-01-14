#include "gridtextctrl.h"

#include <wx/artprov.h>

#include "workbookbase.h"
#include "worksheetbase.h"
#include "rangebase.h"


#define IF_SKIP_RET(cond) \
	if(cond){event.Skip(); return;}



namespace grid
{
    IMPLEMENT_DYNAMIC_CLASS(CGridTextCtrl, wxTextCtrl);

    CGridTextCtrl::CGridTextCtrl(
        wxWindow* parent,
        CWorkbookBase* workbook,
        wxWindowID id,
        wxBitmap bmp,
        const wxColor& ListeningSel,
        const wxColor& NotListeningSel) :wxControl(parent, wxID_ANY)
    {
        m_PrntWnd = parent;
        m_Workbook = workbook;

        m_ListeningSel = ListeningSel;
        m_NOT_ListeningSel = NotListeningSel;

        m_Btn = new wxBitmapButton(this, wxID_ANY, !bmp.IsOk() ? wxArtProvider::GetBitmap(wxART_GO_UP) : bmp);
        m_Txt = new wxTextCtrl(this, wxID_ANY);


        Bind(wxEVT_PAINT, &CGridTextCtrl::OnPaint, this);

        m_Txt->Bind(wxEVT_SET_FOCUS, &CGridTextCtrl::OnSetFocus, this);
        m_Txt->Bind(wxEVT_KILL_FOCUS, &CGridTextCtrl::OnKillFocus, this);

        m_Btn->Bind(wxEVT_BUTTON, &CGridTextCtrl::OnButtonClicked, this);
        m_Btn->Enable(false);
    }


    CGridTextCtrl::~CGridTextCtrl()
    {
        Unbind(wxEVT_PAINT, &CGridTextCtrl::OnPaint, this);

        m_Btn->Unbind(wxEVT_BUTTON, &CGridTextCtrl::OnButtonClicked, this);
        m_Txt->Unbind(wxEVT_SET_FOCUS, &CGridTextCtrl::OnSetFocus, this);
        m_Txt->Unbind(wxEVT_KILL_FOCUS, &CGridTextCtrl::OnKillFocus, this);
    }


    wxSize CGridTextCtrl::DoGetBestSize() const
    {
        wxSize sz = m_Txt->GetBestSize();
        sz.y *= 1.2;

        return sz;
    }


    void CGridTextCtrl::OnPaint(wxPaintEvent& event)
    {
        wxPaintDC dc(this);
        wxSize sz = GetClientSize();

        wxPoint TL = GetClientRect().GetTopLeft();

        int w = sz.GetWidth();
        int h = sz.GetHeight();

        m_Txt->SetSize(wxSize(0.85 * w, h));
        m_Btn->SetSize(wxSize(0.15 * w, h));
        m_Btn->SetPosition(wxPoint(TL.x + 0.85 * w, TL.y));
    }


    wxString CGridTextCtrl::GetRangeText(wxGridRangeSelectEvent& event)
    {
        if (event.Selecting() == false)
            return wxEmptyString;

        auto ws = m_Workbook->GetActiveWS();

        wxString selStr;
        selStr << ws->GetWSName() << "!";
        selStr << CRangeBase::CoordsToStr(event.GetTopLeftCoords(), event.GetBottomRightCoords());

        return selStr;
    }


    void CGridTextCtrl::OnGridRangeSelecting(wxGridRangeSelectEvent& event)
    {
        wxString RngTxt = GetRangeText(event);
        if (!RngTxt.empty())
            m_Txt->SetValue(RngTxt);
    }


    void CGridTextCtrl::OnGridRangeSelected(wxGridRangeSelectEvent& event)
    {
        wxString RngTxt = GetRangeText(event);
        if (!RngTxt.empty())
            m_Txt->SetValue(RngTxt);

        m_Txt->Refresh();
    }



    void CGridTextCtrl::FloatFrm_OnClose(wxCloseEvent& event)
    {
        m_FloatFrm->Unbind(wxEVT_CLOSE_WINDOW, &CGridTextCtrl::FloatFrm_OnClose, this);

        if (!m_FloatFrm->GetValue().empty())
            m_Txt->SetValue(m_FloatFrm->GetValue());

        m_FloatFrm = nullptr;

        m_TopLevelWnd->Show();
        m_TopLevelWnd->Raise();

        m_Txt->SetFocus();

        event.Skip();
    }


    void CGridTextCtrl::OnButtonClicked(wxCommandEvent& event)
    {
        if (m_TopLevelWnd == nullptr)
        {
            m_TopLevelWnd = m_PrntWnd;

            while (!m_TopLevelWnd->IsTopLevel())
                m_TopLevelWnd = m_TopLevelWnd->GetParent();
        }

        m_TopLevelWnd->Show(false);

        if (!m_FloatFrm)
        {
            m_FloatFrm = new GridTextFloatingFrame(this, m_Workbook);
            m_FloatFrm->Bind(wxEVT_CLOSE_WINDOW, &CGridTextCtrl::FloatFrm_OnClose, this);
            m_FloatFrm->Show();
        }
    }



    void CGridTextCtrl::OnSetFocus(wxFocusEvent& evt)
    {
        m_Btn->Enable(true);
        m_Txt->SetBackgroundColour(m_ListeningSel);

        auto ws = m_Workbook->GetActiveWS();
        ws->Bind(wxEVT_GRID_RANGE_SELECTING, &CGridTextCtrl::OnGridRangeSelecting, this);
        ws->Bind(wxEVT_GRID_RANGE_SELECTED, &CGridTextCtrl::OnGridRangeSelected, this);

        //skip so that cursor can be shown and proper focus is visible
        evt.Skip();
    }



    void CGridTextCtrl::OnKillFocus(wxFocusEvent& event)
    {
        wxWindow* TargetWindow = event.GetWindow();

        IF_SKIP_RET(!TargetWindow);

        //not sure what is the targetwindow here but its parent is Worksheet
        IF_SKIP_RET(TargetWindow->GetParent()->IsKindOf(wxCLASSINFO(CWorksheetBase)));

        IF_SKIP_RET(TargetWindow == m_Btn);

        m_Txt->SetBackgroundColour(m_NOT_ListeningSel);
        m_Btn->Enable(false);

        auto ws = m_Workbook->GetActiveWS();

        ws->Unbind(wxEVT_GRID_RANGE_SELECTING, &CGridTextCtrl::OnGridRangeSelecting, this);
        ws->Unbind(wxEVT_GRID_RANGE_SELECTED, &CGridTextCtrl::OnGridRangeSelected, this);

        event.Skip();
    }






    /*******************    GridTextFloatingFrame  ******************************/

    GridTextFloatingFrame::GridTextFloatingFrame(wxWindow* parent, CWorkbookBase* workbook) :
        wxMiniFrame(parent, wxID_ANY, "Select Data", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxSTAY_ON_TOP | wxRESIZE_BORDER)
    {
        SetSizeHints(wxDefaultSize, wxDefaultSize);

        auto Sibling = parent->GetPrevSibling();
        if (auto Txt = wxDynamicCast(Sibling, wxStaticText))
            SetTitle(Txt->GetLabel());

        auto bSizer = new wxBoxSizer(wxVERTICAL);

        m_Txt = new CGridTextCtrl(this, workbook, wxID_ANY, wxArtProvider::GetBitmap(wxART_GO_DOWN));
        m_Txt->SetFocus();
        bSizer->Add(m_Txt, 0, wxALL | wxEXPAND, 5);

        auto Btn = m_Txt->GetButton();
        Btn->SetToolTip("After making a selection click to close and go back to previous top-level dialog");
        Btn->Bind(wxEVT_BUTTON, [&](wxCommandEvent& e) { Close(); });

        auto Sz = bSizer->GetSize();
        Sz.x = Sz.x < 400 ? 400 : Sz.x;
        bSizer->SetMinSize(Sz);
        SetSizerAndFit(bSizer);
        Layout();

        Centre(wxBOTH);
    }
}