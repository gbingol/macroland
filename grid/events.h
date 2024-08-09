#pragma once

#include <wx/wx.h>

#include "dllimpexp.h"

//MarkDirt for CWorksheetBase called
DLLGRID wxDECLARE_EVENT(ssEVT_WS_DIRTY, wxCommandEvent);

//MarkDirty called
DLLGRID wxDECLARE_EVENT(ssEVT_WB_DIRTY, wxCommandEvent);

//MarkClean called
DLLGRID wxDECLARE_EVENT(ssEVT_WB_CLEAN, wxCommandEvent);

//undo or redo event happened (PushUndoEvent called)
DLLGRID wxDECLARE_EVENT(ssEVT_WB_UNDOREDO, wxCommandEvent);