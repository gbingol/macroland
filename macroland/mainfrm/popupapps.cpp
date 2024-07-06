#include "popupapps.h"

#include "../lua/panel.h"
#include "../lua/buttons.h"

#include "frmmacroland.h"



CPopupApps::CPopupApps(frmMacroLand* parent, const wxPoint& Position) :
	wxPopupTransientWindow(parent)
{
	m_Parent = parent;

	m_szrPanel = new wxBoxSizer(wxVERTICAL);

	SetSizer(m_szrPanel);
	Layout();

	SetBackgroundColour(wxColour(225, 232, 232));
}



void CPopupApps::Append(const lua::CPanel& panel)
{
	auto sbSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY,panel.GetTitle()), wxHORIZONTAL);
	const auto& list = panel.GetElementList();

	for (const auto& btn : list)
	{
		int btnID = btn->GetId();
		wxString Title = btn->GetTitle();
		wxBitmap bmp = btn->GetBitmap(btn->GetImagePath());

		auto bmpBtn = new wxBitmapButton(sbSizer->GetStaticBox(), btnID, bmp);
		bmpBtn->SetToolTip(Title);
		bmpBtn->Bind(wxEVT_BUTTON, &lua::CButtonBase::OnClick, (lua::CButtonBase*)btn, btnID);

		sbSizer->Add(bmpBtn, 0, wxALL, 5);
	}

	m_szrPanel->Add(sbSizer, 0, wxEXPAND, 5);


	SetSizer(m_szrPanel);
	Layout();
}



void CPopupApps::OnDismiss()
{
	Hide();
}
