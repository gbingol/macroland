#include "lua_guielements.h"

#include <Python.h>
#include <script/scripting_funcs.h>


namespace {
	namespace fs = std::filesystem;
}




namespace lua
{
	fs::path CElement::s_RootPath = fs::path();

	fs::path CElement::GetNormalizedPath(const fs::path& Path)
	{
		if (!Path.has_root_path())
			return s_RootPath / Path;

		return Path;
	}

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
		m_ImgPath = GetNormalizedPath(Path);
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

	void CButtonBase::SetScriptPath(const fs::path& FilePath)
	{
		if (FilePath.empty())
		{
			m_ScriptPath = FilePath;
			m_IsOK = false;
			return;
		}

		m_ScriptPath = GetNormalizedPath(FilePath);
		m_IsOK = fs::exists(m_ScriptPath);
	}



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

	CToolBarHybridButton::CToolBarHybridButton(CButtonBase* mainBtn):CElement(mainBtn->GetTitle())
	{	
		m_MainBtn = mainBtn;	
	}


	void CToolBarHybridButton::OnHybridButton(wxAuiToolBarEvent& event)
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

			menu.Bind(wxEVT_MENU, &CToolBarHybridButton::OnMenuHandler, this, button->GetId());
		}

		GetWindow()->PopupMenu(&menu);
	}


	void CToolBarHybridButton::OnMenuHandler(wxCommandEvent& event)
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

	CButton::CButton(const std::wstring& Title) :CButtonBase(Title)
	{
	}

	void CButton::OnClick(wxCommandEvent& event)
	{
		auto gstate = PyGILState_Ensure();
		
		if (!m_ScriptPath.empty())
			script::RunPyFile(m_ScriptPath);

		if (!m_ModulePath.empty())
			script::RunPyFunc(m_ModulePath, m_FuncName, m_Param);

		PyErr_Clear();

		PyGILState_Release(gstate);
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


	/************************************************************************** */

	CPanel::CPanel(const std::wstring& title) :CElement(title)
	{
		m_Title = title;
	}


	/************************************************************ */

	CToolBarPage::CToolBarPage(wxWindow* parent, const std::wstring& title) :wxPanel(parent, wxID_ANY)
	{
		m_Title = title;

		m_ToolBar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_HORIZONTAL | wxAUI_TB_TEXT);

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
		wxNotebook::AddPage(page, label);

		auto TB = page->GetToolBar();

		auto Elems = page->GetElementList();
		for (auto& elem : Elems)
		{
			int btnID = elem->GetId();
			wxString Title = elem->GetTitle();
			wxBitmap bmp = elem->GetBitmap(elem->GetImagePath());
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

				auto MainBtn = ((CToolBarHybridButton*)elem)->GetMainButton();
				TB->AddTool(btnID, MainBtn->GetTitle(), MainBtn->GetBitmap(MainBtn->GetImagePath()));
				TB->SetToolDropDown(btnID, true);
				
				TB->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &CToolBarHybridButton::OnHybridButton, (CToolBarHybridButton*)elem, btnID);
			}
		}

		TB->Realize();
	}


	CToolBarPage* CToolBarNtbk::FindPage(const std::wstring& label)
	{
		size_t N = GetPageCount();
		for (size_t i = 0; i < N; ++i)
		{
			auto Page = GetPage(i);
			if (Page->GetLabel() == label)
				return (CToolBarPage*)Page;
		}

		return nullptr;
	}

}