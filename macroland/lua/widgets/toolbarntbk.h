#pragma once

#include <string>
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>
#include <wx/notebook.h>

#include "baseelement.h"


namespace lua
{
	class CToolBarPage : public wxPanel
	{
	public:
		enum class PAGETYPE
		{
			NONE = 0, //no specific type
			EXTENSION //is an extension
		};
	public:
		CToolBarPage(wxWindow* parent, const std::wstring& title);

		auto GetElementList() const 
		{
			return m_ElemList;
		}

		void AddElement(CElement* elem)
		{
			m_ElemList.push_back(elem);
		}

		auto GetPageType() const {
			return m_PageType;
		}

		void SetPageType(PAGETYPE type) 
		{
			m_PageType = type;
		}

		auto GetExtensionPath() const {
			return m_ExtensionPath;
		}

		void SetExtensionPath(const std::filesystem::path& path) 
		{
			m_ExtensionPath = path;
		}

		auto GetToolBar() const
		{
			return m_ToolBar;
		}

		auto GetTitle() const
		{
			return m_Title;
		}


	private:
		std::list<CElement*> m_ElemList;
		PAGETYPE m_PageType;
		std::filesystem::path m_ExtensionPath;
		wxAuiToolBar* m_ToolBar;
		std::wstring m_Title;
	};



	/*************************************************************/

	class CToolBarNtbk : public wxNotebook
	{
	public:
		CToolBarNtbk(wxWindow* parent, int ID = wxID_ANY);
		~CToolBarNtbk();

		CToolBarPage* AddPage(const std::wstring& label);
		void AddPage(CToolBarPage* page);

		CToolBarPage* FindPage(const std::wstring& label);
	};

}