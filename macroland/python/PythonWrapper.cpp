#include "PythonWrapper.h"


namespace Python
{
   
    PyObject* Worksheet_FromCWorksheet(ICELL::CWorksheet* ws)
    {
        auto WorksheetObj = PyObject_New(Worksheet, &PythonWorksheet_Type);
        WorksheetObj->ptrObj = ws;
        WorksheetObj->state = true;

        return (PyObject*)WorksheetObj;
    }


}