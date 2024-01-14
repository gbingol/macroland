#include "menuelem.h"


namespace lua
{
    wxMenu* CMenu::GetAsMenu() const
    {
		auto Menu = new wxMenu();
		for (auto btn : GetList())
		{
			int btnID = btn->GetId();
			wxString Title = btn->GetTitle();
			wxBitmap bmp = btn->GetBitmap(btn->GetImagePath());

			auto Item = Menu->Append(btnID, Title);
			Item->SetBitmap(bmp);

			Menu->Bind(wxEVT_MENU, &CButtonBase::OnClick, (CButtonBase*)btn, btnID);
		}

		return Menu;
    }
}