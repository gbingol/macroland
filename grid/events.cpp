#include "events.h"

wxDEFINE_EVENT(ssEVT_GRID_DIRTY, wxCommandEvent);
wxDEFINE_EVENT(ssEVT_WORKBOOK_DIRTY, wxCommandEvent);
wxDEFINE_EVENT(ssEVT_WORKBOOK_CLEAN, wxCommandEvent);

wxDEFINE_EVENT(ssEVT_WORKBOOK_UNDOREDOSTACKCHANGED, wxCommandEvent);