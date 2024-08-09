#include "events.h"


//Events for worksheet
wxDEFINE_EVENT(ssEVT_WS_DIRTY, wxCommandEvent);

//Events for workbook
wxDEFINE_EVENT(ssEVT_WB_DIRTY, wxCommandEvent);
wxDEFINE_EVENT(ssEVT_WB_CLEAN, wxCommandEvent);
wxDEFINE_EVENT(ssEVT_WB_UNDOREDO, wxCommandEvent); //Undo-redo stack changed