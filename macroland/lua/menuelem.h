#pragma once

#include <string>
#include <wx/wx.h>

#include "buttons.h"


namespace lua
{
	class CMenu : public CButtonCollection
	{
	public:
		CMenu(const wxString& Title) :CButtonCollection(Title) {}

		Type GetType() const override {
			return CElement::Type::DropButton;
		}

		wxMenu* GetAsMenu() const;
	};

}