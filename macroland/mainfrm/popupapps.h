#pragma once

#include <wx/wx.h>
#include <wx/popupwin.h>
#include <wx/statline.h>


namespace lua
{
	class CPanel;
}


class frmMacroLand;

class CPopupApps : public wxPopupTransientWindow
{
public:
	CPopupApps(frmMacroLand* parent, const wxPoint& Position);

	void Append(const lua::CPanel& panel);

protected:
	wxBoxSizer *m_szrPanel;

	void OnDismiss() override;

private:
	frmMacroLand* m_Parent;
};
