#include "guielements.h"

#include <Python.h>
#include "../cmdwnd/scripting_funcs.h"


namespace {
	namespace fs = std::filesystem;
}




namespace extension
{
	

	wxBitmap CElement::GetBitmap(const fs::path& BitmapPath)
	{
		wxBitmap bmp;

		if (BitmapPath.empty())
			return bmp;

		if (!bmp.LoadFile(BitmapPath.wstring(), wxBITMAP_TYPE_ANY))
			throw std::runtime_error("Image at " + BitmapPath.string() + " could not be loaded.");

		if (bmp.GetHeight() > 64 || bmp.GetWidth() > 64)
			throw std::runtime_error("Image at" + BitmapPath.string() + " is larger than 64 by 64 pixels.");

		return bmp;
	}


	void CElement::SetImgPath(const fs::path& Path)
	{
		m_ImgPath = Path;
	}



	/******************************************************************/

	void CToolBarDropButton::OnClick(wxCommandEvent& event)
	{
		wxMenu Menu;
		AttachToMenu(&Menu);
		GetWindow()->PopupMenu(&Menu);
	}




	/*****************************   Button   ******************************************/

	CButtonBase::CButtonBase(const std::wstring& Title) :CElement(Title) { }




	/******************** BUTTON COLLECTION ***********************/

	void CButtonCollection::OnMenuHandler(wxCommandEvent& event)
	{
		auto btnList = GetList();

		for (auto button : btnList)
			if (button->GetId() == event.GetId())
				button->OnClick(event);
	}


	void CButtonCollection::AttachToMenu(wxMenu* menu)
	{
		for (const auto& button : GetList())
		{
			std::filesystem::path bmppath = button->GetImagePath();
			wxBitmap bmp = GetBitmap(bmppath);

			auto MenuItem = menu->Append(button->GetId(), button->GetTitle());
			if (bmp.IsOk())
				MenuItem->SetBitmap(bmp);

			menu->Bind(wxEVT_MENU, &CButtonCollection::OnMenuHandler, this, button->GetId());
		}
	}


	/*****************************************************************/

	CHybridButton::CHybridButton(CButtonBase* mainBtn):CElement(mainBtn->GetTitle())
	{	
		m_MainBtn = mainBtn;	
	}


	void CHybridButton::OnHybridButton(wxAuiToolBarEvent& event)
	{
		if (!event.IsDropDownClicked())
		{
			m_MainBtn->OnClick(event);
			return;
		}

		wxMenu menu;
		for (const auto& button : GetButtonList())
		{
			std::filesystem::path bmppath = button->GetImagePath();
			wxBitmap bmp = GetBitmap(bmppath);

			auto MenuItem = menu.Append(button->GetId(), button->GetTitle());
			if (bmp.IsOk())
				MenuItem->SetBitmap(bmp);

			menu.Bind(wxEVT_MENU, &CHybridButton::OnMenuHandler, this, button->GetId());
		}

		GetWindow()->PopupMenu(&menu);
	}


	void CHybridButton::OnMenuHandler(wxCommandEvent& event)
	{
		auto btnList = GetButtonList();

		for (const auto& button : btnList)
		{
			if (button->GetId() != event.GetId())
				continue;

			//call the button's event
			button->OnClick(event);

			//add the MainButton to button's list
			m_Btns.push_back(m_MainBtn);

			//Change main button to the button (the selected menu button)
			m_MainBtn = button;

			//erase the button (menu button) from buttons list
			std::erase(m_Btns, button);

			//if dynamic_cast succeeds change the image and label to the selected menu button's
			if (dynamic_cast<wxAuiToolBar*>(GetWindow()))
			{
				auto TB = (wxAuiToolBar*)GetWindow();
				TB->SetToolBitmap(GetId(), button->GetBitmap(button->GetImagePath()));
				TB->SetToolLabel(GetId(), button->GetTitle());
			}
		}
	}



	/*********************************************************************/

	CButton::CButton(const std::wstring& Title) :CButtonBase(Title)	{ }

	void CButton::OnClick(wxCommandEvent& event)
	{
		auto gstate = cmdedit::GILStateEnsure();

		PyObject_CallObject(m_Func, m_Args);
	}



	/************************************************************************ */


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



	/************************************************************ */

	CToolBarPage::CToolBarPage(wxWindow* parent, const std::wstring& title) :wxPanel(parent, wxID_ANY)
	{
		m_Title = title;

		m_ToolBar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_OVERFLOW|wxAUI_TB_HORIZONTAL | wxAUI_TB_TEXT);

		auto szrMain = new wxBoxSizer(wxVERTICAL);
		szrMain->Add(m_ToolBar, 0, wxEXPAND | wxALL, 1);
		SetSizer(szrMain);
		Layout();
	}



	/***************************************************************/

	CToolBarNtbk::CToolBarNtbk(wxWindow* parent, int ID) :
		wxNotebook(parent, ID, wxDefaultPosition, wxDefaultSize, 0) { }

	CToolBarNtbk::~CToolBarNtbk() = default;


	CToolBarPage* CToolBarNtbk::AddPage(const std::wstring& label)
	{
		auto panel = new CToolBarPage(this, label);
		wxNotebook::AddPage(panel, label);

		return panel;
	}

	void CToolBarNtbk::AddPage(CToolBarPage* page)
	{
		auto label = page->GetTitle();

		if(!FindPage(label))
			wxNotebook::AddPage(page, label);

		auto TB = page->GetToolBar();

		auto Elems = page->GetElementList();
		for (auto& elem : Elems)
		{
			int btnID = elem->GetId();
			auto Title = elem->GetTitle();
			auto bmp = elem->GetBitmap(elem->GetImagePath());
			auto Type = elem->GetType();

			if (Type == CElement::Type::Button)
			{
				TB->AddTool(btnID, Title, bmp);
				TB->Bind(wxEVT_TOOL, &CButtonBase::OnClick, (CButtonBase*)elem, btnID);
			}
			else if (Type == CElement::Type::DropButton)
			{
				elem->SetWindow(TB);

				TB->AddTool(btnID, Title, bmp);
				TB->Bind(wxEVT_TOOL, &CToolBarDropButton::OnClick, (CToolBarDropButton*)elem, btnID);
			}

			else if (Type == CElement::Type::HybridBtn)
			{
				elem->SetWindow(TB);

				auto MainBtn = ((CHybridButton*)elem)->GetMainButton();
				TB->AddTool(btnID, MainBtn->GetTitle(), MainBtn->GetBitmap(MainBtn->GetImagePath()));
				TB->SetToolDropDown(btnID, true);
				
				TB->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CHybridButton::OnHybridButton, (CHybridButton*)elem, btnID);
			}
		}

		TB->Realize();
	}


	CToolBarPage* CToolBarNtbk::FindPage(const std::wstring& label)
	{
		size_t N = GetPageCount();
		for (size_t i = 0; i < N; ++i)
		{
			auto PgTxt = GetPageText(i);
			if (PgTxt == label)
				return (CToolBarPage*)GetPage(i);
		}

		return nullptr;
	}

}