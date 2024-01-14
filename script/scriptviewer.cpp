#include "scriptviewer.h"

#include <wx/gdicmn.h> 

namespace script
{
	frmSriptViewer::frmSriptViewer(
		wxWindow* parent, 
		const std::filesystem::path& ScriptPath, 
		std::optional<size_t> FirstVisibleLine):
		wxMiniFrame(parent, wxID_ANY, ScriptPath.wstring(), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
	{
		SetSizeHints(wxDefaultSize, wxDefaultSize);
		auto bSizer = new wxBoxSizer(wxVERTICAL);

		m_Txt = new CStyledTextCtrl(this); 
		m_Txt->SetViewWhiteSpace(0);

		try
		{
			m_Txt->LoadFile(ScriptPath);
		}
		catch(const std::exception& e)
		{
			wxMessageBox(e.what());
		}

		if (FirstVisibleLine.has_value())
			m_Txt->SetFirstVisibleLine(FirstVisibleLine.value());
	
		bSizer->Add(m_Txt, 1, wxEXPAND | wxALL, 5);

		SetSizerAndFit(bSizer);
		Layout();

		wxSize screen = wxGetDisplaySize();
		SetSize(wxSize(screen.x / 2, screen.y * 0.6));
	}
}
