#include "panel.h"

#include "buttons.h"



namespace lua
{
	CPanel::CPanel(const std::wstring& title) :CElement(title)
	{
		m_Title = title;
	}
}