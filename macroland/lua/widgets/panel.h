#pragma once

#include <string>

#include "baseelement.h"


namespace lua
{
	class CPanel : public CElement
	{
	public:
		CPanel(const std::wstring& title);

		Type GetType() const
		{
			return Type::Panel;
		}

		auto GetElementList() const 
		{
			return m_ElemList;
		}

		void AddElement(CElement* elem)
		{
			m_ElemList.push_back(elem);
		}


		auto GetExtPath() const {
			return m_ExtensionPath;
		}

		void SetExtensionPath(const std::filesystem::path& path) 
		{
			m_ExtensionPath = path;
		}


		auto GetTitle() const
		{
			return m_Title;
		}


	private:
		std::list<CElement*> m_ElemList;
		std::filesystem::path m_ExtensionPath;
		std::wstring m_Title;
	};

}