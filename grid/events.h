#pragma once

#include <wx/wx.h>

#include "dllimpexp.h"

//MarkDirt for CWorksheetBase called
DLLGRID wxDECLARE_EVENT(ssEVT_GRID_DIRTY, wxCommandEvent);

//MarkDirty called
DLLGRID wxDECLARE_EVENT(ssEVT_WORKBOOK_DIRTY, wxCommandEvent);

//MarkClean called
DLLGRID wxDECLARE_EVENT(ssEVT_WORKBOOK_CLEAN, wxCommandEvent);

//undo or redo event happened (PushUndoEvent called)
DLLGRID wxDECLARE_EVENT(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, wxCommandEvent);