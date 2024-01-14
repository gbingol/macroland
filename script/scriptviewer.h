#pragma once

#include <filesystem>
#include <optional>

#include <wx/wx.h>
#include <wx/minifram.h>

#include "styledtxtctrl.h"

#include "dllimpexp.h"


namespace script
{
	class frmSriptViewer : public wxMiniFrame 
	{
	public: 
		DLLSCRIPT frmSriptViewer(
			wxWindow* parent, 
			const std::filesystem::path& ScriptPath,
			std::optional<size_t> FirstVisibleLine = std::nullopt);

		DLLSCRIPT ~frmSriptViewer() = default;

	private:
		CStyledTextCtrl* m_Txt;
	};
}

