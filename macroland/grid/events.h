#pragma once

#include <wx/wx.h>



//MarkDirt for CWorksheetBase called
wxDECLARE_EVENT(ssEVT_WS_DIRTY, wxCommandEvent);

//MarkDirty called
wxDECLARE_EVENT(ssEVT_WB_DIRTY, wxCommandEvent);

//MarkClean called
wxDECLARE_EVENT(ssEVT_WB_CLEAN, wxCommandEvent);

//undo or redo event happened (PushUndoEvent called)
wxDECLARE_EVENT(ssEVT_WB_UNDOREDO, wxCommandEvent);