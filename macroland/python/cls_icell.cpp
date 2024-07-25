
#include <string>
#include <vector>
#include <complex>
#include <memory>
#include <wx/wx.h>
#include <sstream>

#include <script/scripting_funcs.h>

#include "../icell/workbook.h"
#include "../icell/worksheet.h"

#include "PythonWrapper.h"



extern ICELL::CWorkbook* glbWorkbook;


using namespace script;


#define CHECKSTATE(OBJ, RET)					                                        \
if ((OBJ)->state == false)                                                      \
{                                                                               \
    PyErr_SetString(PyExc_ReferenceError, "Object is not usable anymore");   \
                                                                                \
    return (RET);                                                             \
}


static std::tuple<int, int, int> rgb(const std::string & str)
{
    int R = 0, G = 0, B = 0;
    std::stringstream ss(str);

    int c; int pos = 0;
    while (ss >> c)
    {
        if (c > 255 || c < 0)
            throw std::exception("RGB components must be in the range of 0..255");

        if (pos == 0) R = c;
        if (pos == 1) G = c;
        if (pos == 2) B = c;

        pos++;
    }

    if (pos != 3)
        throw std::exception("Exactly 3 components (R G B) required");

    return { R, G, B };
}



/*********************************************************************/
static PyObject* range_clear(Python::Range * self)
{
    CHECKSTATE(self, nullptr);

    self->ptrObj->clear();

    Py_RETURN_NONE;
}



static PyObject* range_col(Python::Range* self, PyObject* args)
{  
    CHECKSTATE(self, nullptr);

    int Pos = -1;

    if (!PyArg_ParseTuple(args, "i", &Pos))
        return nullptr;

    if (Pos < 0 || Pos >= self->ptrObj->ncols())
    {
        PyErr_SetString(PyExc_IndexError, "Requested column is out of range boundaries.");
        return nullptr;
    }

    auto ColRng = (ICELL::CRange*)self->ptrObj->col(Pos);
    auto Arr = ColRng->toArrays(0)[0];

    return Python::List_FromVectorString(Arr);
}



static PyObject* range_coords(Python::Range* self)
{
    CHECKSTATE(self, nullptr);

    auto TL = self->ptrObj->topleft();
    auto BR = self->ptrObj->bottomright();

    
    auto TLObj = PyTuple_New(2);
    PyTuple_SetItem(TLObj, 0, Py_BuildValue("i", TL.GetRow()));
    PyTuple_SetItem(TLObj, 1, Py_BuildValue("i", TL.GetCol()));

    auto BRObj = PyTuple_New(2);
    PyTuple_SetItem(BRObj, 0, Py_BuildValue("i", BR.GetRow()));
    PyTuple_SetItem(BRObj, 1, Py_BuildValue("i", BR.GetCol()));


    auto TupleObj = PyTuple_New(2);

    PyTuple_SetItem(TupleObj, 0, TLObj);
    PyTuple_SetItem(TupleObj, 1, BRObj);

    return TupleObj;
}



static PyObject* range_isOK(Python::Range* self)
{
    bool IsOK = self->state;

    if (IsOK)
    {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}



static PyObject* range_ncols(Python::Range* self)
{
    CHECKSTATE(self, nullptr);

    return Py_BuildValue("i", self->ptrObj->ncols());
}



static PyObject* range_nrows(Python::Range* self)
{
    CHECKSTATE(self, nullptr);
    
    return Py_BuildValue("i", self->ptrObj->nrows());
}



static PyObject* range_parent(Python::Range* self)
{
    CHECKSTATE(self, nullptr);

    auto Parent = self->ptrObj->GetWorksheet();

    return Python::Worksheet_FromCWorksheet(Parent);
}


static PyObject* range_select(Python::Range* self)
{
    CHECKSTATE(self, nullptr);

    auto ws = self->ptrObj->GetWorksheet();

    ws->GetParent()->GetParent()->Raise();
    ws->SelectBlock(self->ptrObj->topleft(), self->ptrObj->bottomright());

    Py_RETURN_NONE;
}



static PyObject* range_subrange(Python::Range* self, PyObject* args, PyObject* kwargs)
{
    CHECKSTATE(self, nullptr);

    int Row, Col;
    int NRows = -1, NCols = -1;


    const char* kwlist[] = { "row","col", "nrows", "ncols", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii|ii", const_cast<char**>(kwlist), &Row, &Col, &NRows, &NCols))
        return nullptr;

    CHECKNONNEGATIVE_RET(Row, "row must be >= 0");
    CHECKNONNEGATIVE_RET(Col, "col must be >= 0");

    try {
        auto retRange = (ICELL::CRange*)self->ptrObj->GetSubRange(wxGridCellCoords(Row, Col), NRows, NCols);

        if (!retRange)
            return Py_BuildValue("");

        return Python::Range_FromCRange(retRange);
    }
    CATCHRUNTIMEEXCEPTION_RET();

    Py_RETURN_NONE;
}


static PyObject* range_todict(Python::Range* self, PyObject* args, PyObject* kwargs)
{
    CHECKSTATE(self, nullptr);
    auto SelfRng = self->ptrObj;

    bool HasHeaders = true;
    PyObject* HeaderObj = nullptr;

    const char* kwlist[] = { "headers", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", const_cast<char**>(kwlist), &HeaderObj))
        return nullptr;

    if (HeaderObj)
    {
        IF_PYERR_RET(!IsExactTypeBool(HeaderObj), PyExc_TypeError, "headers must be boolean");
        HasHeaders = Py_IsTrue(HeaderObj);
    }
   
    try
    {
        auto Arrs = SelfRng->toArrays(0);

        PyObject* Dict = PyDict_New();  
        for (size_t i = 0; auto & Arr: Arrs)
        {
            wxString Header;
            if (HasHeaders)
            {
                Header = Arr[0];
                Arr.erase(Arr.begin());
            }
            else
            {
                Header << "Col" << (i + 1);
                i++;
            }

            PyDict_SetItemString(Dict, Header.mb_str(wxConvUTF8), Python::List_FromVectorString(Arr));
        }

        return Dict;
    }
    CATCHRUNTIMEEXCEPTION_RET();
   

    Py_RETURN_NONE;
}




static PyObject* range_tolist(Python::Range* self, PyObject* args, PyObject* kwargs)
{
    CHECKSTATE(self, nullptr);

    auto SelfRng = self->ptrObj;

    int Axis = -1;

    const char* kwlist[] = { "axis", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", const_cast<char**>(kwlist), &Axis))
        return nullptr;

    if (Axis == -1)
    {
        auto Arr = SelfRng->toArray();
        return Python::List_FromVectorString(std::move(Arr));
    }
    else
    {
        try
        {
            auto Arrs = SelfRng->toArrays(Axis);

            PyObject* RetList = PyList_New(Arrs.size());
            for (size_t i = 0; const auto & Arr: Arrs)
                PyList_SetItem(RetList, i++, Python::List_FromVectorString(Arr));

            return RetList;
        }
        CATCHRUNTIMEEXCEPTION_RET();
    }

    Py_RETURN_NONE;
}



static PyMethodDef PyRange_methods[] =
{
    {"clear",
    (PyCFunction)range_clear,
    METH_NOARGS,
    "clears the content and format, clear()"},

    {"col", 
    (PyCFunction)range_col,
    METH_VARARGS, 
    "returns the column as Python list,  col(pos=)->list"},

    {"coords", 
    (PyCFunction)range_coords,
    METH_VARARGS, 
    "returns the top-left and bottom-right coordinates, coords()-> tuple, tuple"},

    {"isOK", 
    (PyCFunction)range_isOK,
    METH_NOARGS, 
    "return if Range is still usable or not, isOK()->bool"},

    {"ncols", 
    (PyCFunction)range_ncols,
    METH_NOARGS, 
    "returns the number of columns, ncols()->int"},

    {"nrows", 
    (PyCFunction)range_nrows,
    METH_NOARGS, 
    "returns the number of rows, nrows()->int"},

    {"parent", 
    (PyCFunction)range_parent,
    METH_NOARGS, 
    "returns the Worksheet which owns the Range, parent()->Worksheet"},

    {"select", 
    (PyCFunction)range_select,
    METH_NOARGS, 
    "selects the range, select()"},

    {"subrange", 
    (PyCFunction)range_subrange,
    METH_VARARGS | METH_KEYWORDS, 
    "returns a range defined within current range, subrange(row=, col=, nrows=-1, ncols=-1)->Range"},


    {"tolist",
    (PyCFunction)range_tolist,
    METH_VARARGS | METH_KEYWORDS,
    "returns the whole range as list, tolist([axis = ])-> 1D/2D list"},

    {"todict",
    (PyCFunction)range_todict,
    METH_VARARGS | METH_KEYWORDS,
    "returns the whole range as dict, todict([headers = True])"},

    {NULL}  /* Sentinel */
};





/********************** Methods Used By type/mapping/sequence/nb **************************/


static int Range_init(Python::Range* self, PyObject* args)
{
    const char* RangeStr = "";

    if (!PyArg_ParseTuple(args, "s", &RangeStr))
        return -1;

    try
    {
        auto rng = new ICELL::CRange(wxString::FromUTF8(RangeStr), glbWorkbook);

        self->ptrObj = rng;
        self->state = true;
        self->ptrObj->GetWorksheet()->RegisterPyRng(self);
    }
    catch (std::exception& e)
    {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0; 
}



static void Range_dealloc(Python::Range* self)
{
    self->state = false;
}



static Py_ssize_t Range_sequence_len(PyObject* obj)
{
    auto Rng = (Python::Range*)obj;

    CHECKSTATE(Rng, 0);

    return Rng->ptrObj->ncols() * Rng->ptrObj->nrows();
}



static PyObject* Range_mp_subscript(PyObject* self, PyObject* args)
{
    auto SelfRng = (Python::Range*)self;

    CHECKSTATE(SelfRng, nullptr);

    int Row, Col;

    if (!PyArg_ParseTuple(args, "ii", &Row, &Col))
        return nullptr;

    CHECKNONNEGATIVE_RET(Row, "row position must be >=0");
    CHECKNONNEGATIVE_RET(Col, "col position must be >=0");

    try
    {
        const wxString& CellValue = SelfRng->ptrObj->get(Row, Col);
        auto VarObj = Python::Object_FromString(CellValue.ToStdWstring());

        return VarObj;
    }
    CATCHRUNTIMEEXCEPTION_RET();

    Py_RETURN_NONE;
}



static int Range_mp_ass_subscript(PyObject* self, PyObject* key, PyObject* rhs)
{
    auto SelfRng = (Python::Range*)self;

    CHECKSTATE(SelfRng, -1);

    int Row, Col;
    if (!PyArg_ParseTuple(key, "ii", &Row, &Col))
        return -1;

    CHECKNONNEGATIVE_RETTYPE(Row, "row position must be >=0", -1);
    CHECKNONNEGATIVE_RETTYPE(Col, "col position must be >=0", -1);

   
    wxString CellValue = PyUnicode_AsWideCharString(rhs, nullptr);
    try 
    {
        SelfRng->ptrObj->set(Row, Col, CellValue);
    }
    catch (std::exception& e)
    {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return -1;
    }

    return 0;
}



static PyObject* Range_tp_str(PyObject* self)
{
    auto Rng = (Python::Range*)self;

    CHECKSTATE(Rng, nullptr);

    wxString str = Rng->ptrObj->toString();
    return PyUnicode_FromString(str.mb_str(wxConvUTF8));
}



static PyObject* Range_tp_iternext(PyObject* self)
{
    auto Rng = (Python::Range*)self;

    size_t RngSize = Rng->ptrObj->ncols() * Rng->ptrObj->nrows();

    if (Rng->iter < RngSize)
    {
        std::wstring Str = Rng->ptrObj->get(Rng->iter).ToStdWstring();

        Rng->iter++;

        return Python::Object_FromString(Str);
    }
    else
    {
        Rng->iter = 0;
        return nullptr;
    }
}


static PyObject* Range_tp_iter(PyObject* self)
{
    Py_INCREF(self);
    return self;
}


static PySequenceMethods Range_sequence_methods = { (lenfunc)Range_sequence_len};


static PyMappingMethods Range_MappingMethods = { };


PyTypeObject PythonRange_Type = { PyVarObject_HEAD_INIT(NULL, 0) "Range"};



int PyInit_Range(PyObject* Module)
{
    Range_MappingMethods.mp_subscript = Range_mp_subscript;
    Range_MappingMethods.mp_ass_subscript = Range_mp_ass_subscript;


    PythonRange_Type.tp_new = PyType_GenericNew;
    PythonRange_Type.tp_basicsize = sizeof(Python::Range);
    
    PythonRange_Type.tp_dealloc = (destructor)Range_dealloc;
    PythonRange_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PythonRange_Type.tp_doc = "ScienceSuit's built-in Range";

    PythonRange_Type.tp_methods = PyRange_methods;
    PythonRange_Type.tp_init = (initproc)Range_init;
    PythonRange_Type.tp_str = (reprfunc)Range_tp_str;
    PythonRange_Type.tp_iternext = (iternextfunc)Range_tp_iternext;
    PythonRange_Type.tp_iter = (getiterfunc)Range_tp_iter;
    PythonRange_Type.tp_as_sequence = &Range_sequence_methods;
    PythonRange_Type.tp_as_mapping = &Range_MappingMethods;

    if (PyType_Ready(&PythonRange_Type) < 0)
        return -1;

   
    Py_INCREF(&PythonRange_Type);

    if (PyModule_AddObject(Module, "Range", (PyObject*)&PythonRange_Type) < 0)
    {
        Py_DECREF(&PythonRange_Type);
        return -1;
    }

    return 0;
}





/**************************  WORKSHEET     *****************************************/

static PyObject* ws_getvalue(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int row=-1, col=-1;
    const char* kwlist[] = { "row", "col", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", const_cast<char**>(kwlist), &row, &col))
        return nullptr;

    CHECKSTATE(self, nullptr);

    auto WS = self->ptrObj;
    auto Value = WS->GetCellValue(row, col);

    return PyUnicode_FromWideChar(Value.ToStdWstring().c_str(), -1);
}


static PyObject* ws_setvalue(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int row=-1, col=-1;
	PyObject *ValueObj{nullptr};
	const char* kwlist[] = { "row", "col", "value", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiO", const_cast<char**>(kwlist), &row, &col, &ValueObj))
        return nullptr;

    CHECKSTATE(self, nullptr);

	std::string s = PyUnicode_AsUTF8(ValueObj);

	auto WS = self->ptrObj;
    self->ptrObj->SetCellValue(row, col, wxString::FromUTF8(s));

	Py_RETURN_NONE;
}



static PyObject* ws_appendrows(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int N = 1;

    const char* kwlist[] = { "n", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", const_cast<char**>(kwlist), &N))
        return nullptr;

    CHECKSTATE(self, nullptr);

    auto WS = self->ptrObj;
    bool Success = WS->InsertRows(WS->GetNumberRows() - 1, N);

    if (Success)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}



static PyObject* ws_appendcols(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int N = 1;

    const char* kwlist[] = { "n", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", const_cast<char**>(kwlist), &N))
        return nullptr;

    CHECKSTATE(self, nullptr);

    auto WS = self->ptrObj;
    bool Success = WS->InsertCols(WS->GetNumberCols() - 1, N);

    if (Success)
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}



static PyObject* ws_bindFunction(
    Python::Worksheet* self, PyObject* args)
{
    auto SelfWS = (Python::Worksheet*)self;
    CHECKSTATE(SelfWS, nullptr);

    size_t NArgs = PyTuple_GET_SIZE(args);
	std::string str = std::to_string(NArgs) + " provided, at least 2 parameters must be provided";
	IF_PYERRVALUE_RET(NArgs < 2, str.c_str());


    PyObject* EventNameObj = PyTuple_GetItem(args, 0);
    IF_PYERRVALUE_RET(!PyUnicode_Check(EventNameObj), "event type must be a string.");

    std::string EventName = PyUnicode_AsUTF8(EventNameObj);

    PyObject* FuncObj = PyTuple_GetItem(args, 1);
    ASSERT_CALLABLE_RET(FuncObj, "parameter must be of type function");


    size_t NFuncArgs = NArgs - 2;

    Python::CEventCallbackFunc* CallbackFunc = new Python::CEventCallbackFunc();
    CallbackFunc->m_FuncObj = FuncObj;

    PyObject* FuncArgs = nullptr;
    try
    {
        if (NFuncArgs > 0)
        {
            FuncArgs = PyTuple_New(NFuncArgs);

            for (size_t i = 2, j = 0; i < NArgs; ++i, ++j)
                PyTuple_SetItem(FuncArgs, j, PyTuple_GetItem(args, i));
        }

        CallbackFunc->m_FuncArgs = FuncArgs;

        SelfWS->ptrObj->BindPythonFunction(EventName, CallbackFunc);
    }
    catch (std::exception& e)
    {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        Py_XDECREF(FuncArgs);
        delete CallbackFunc;

        return nullptr;
    }

    Py_RETURN_NONE;
}




static PyObject* ws_UnbindFunction(
    Python::Worksheet* self, PyObject* args)
{
    auto SelfWS = (Python::Worksheet*)self;

    CHECKSTATE(SelfWS, nullptr);

    size_t NArgs = PyTuple_GET_SIZE(args);
    IF_PYERRVALUE_RET(NArgs < 2, "At least 2 parameters must be provided");

    PyObject* EventNameObj = PyTuple_GetItem(args, 0);
    IF_PYERRVALUE_RET(!PyUnicode_Check(EventNameObj), "event type must be a string.");

    PyObject* FuncObj = PyTuple_GetItem(args, 1);
    ASSERT_CALLABLE_RET(FuncObj, "parameter must be of type function");

    try
    {
        std::string EventName = PyUnicode_AsUTF8(EventNameObj);
        SelfWS->ptrObj->UnbindPythonFunction(EventName, FuncObj);
    }
    CATCHRUNTIMEEXCEPTION_RET();

    Py_RETURN_NONE;
}



static PyObject* ws_GridCursor(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    auto Coords = self->ptrObj->GetGridCursorCoords();
    int row = Coords.GetRow() >= 0 ? Coords.GetRow() : 0;
    int col = Coords.GetCol() >= 0 ? Coords.GetCol() : 0;

    auto Tuple = PyTuple_New(2);
    PyTuple_SetItem(Tuple, 0, Py_BuildValue("i", row));
    PyTuple_SetItem(Tuple, 1, Py_BuildValue("i", col));

    return Tuple;
}



static PyObject* ws_isOK(Python::Worksheet* self)
{
    if (self->state)
    {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}



static PyObject* ws_name(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    wxString str = self->ptrObj->GetWSName();

    return PyUnicode_FromString(str.mb_str(wxConvUTF8));
}


static PyObject* ws_ncols(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    int N = self->ptrObj->GetNumberCols();

    return Py_BuildValue("i", N);
}


static PyObject* ws_nrows(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    int N = self->ptrObj->GetNumberRows();

    return Py_BuildValue("i", N);
}



static PyObject* ws_selection(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    if (self->ptrObj->IsSelection() == false)
        return Py_BuildValue("");

    auto range = self->ptrObj->GetSelection();
    auto RangeObj = Python::Range_FromCRange(range.release());

    self->ptrObj->RegisterPyRng((Python::Range*)RangeObj);

    return RangeObj;
}


static PyObject* ws_selcoords(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);

    if (self->ptrObj->IsSelection() == false)
        Py_RETURN_NONE;

    auto range = self->ptrObj->GetSelection();
    auto TL = range->topleft();
    auto BR = range->bottomright();

    
    auto TLObj = PyTuple_New(2);
    PyTuple_SetItem(TLObj, 0, Py_BuildValue("i", TL.GetRow()));
    PyTuple_SetItem(TLObj, 1, Py_BuildValue("i", TL.GetCol()));

    auto BRObj = PyTuple_New(2);
    PyTuple_SetItem(BRObj, 0, Py_BuildValue("i", BR.GetRow()));
    PyTuple_SetItem(BRObj, 1, Py_BuildValue("i", BR.GetCol()));


    auto TupleObj = PyTuple_New(2);

    PyTuple_SetItem(TupleObj, 0, TLObj);
    PyTuple_SetItem(TupleObj, 1, BRObj);

    return TupleObj;
}





static PyMethodDef PyWorksheet_methods[] =
{
    { "getvalue",
    (PyCFunction)ws_getvalue,
    METH_VARARGS | METH_KEYWORDS,
    "return the value of a cell-> getvalue(row, col)->str" },

	{ "setvalue",
    (PyCFunction)ws_setvalue,
    METH_VARARGS | METH_KEYWORDS,
    "sets the value of a cell-> setvalue(row, col, value)" },


    { "appendcols",
    (PyCFunction)ws_appendcols,
    METH_VARARGS | METH_KEYWORDS,
    "appends columns, appendcols(n = 1)->bool" },

    { "appendrows",
    (PyCFunction)ws_appendrows,
    METH_VARARGS | METH_KEYWORDS,
    "appends rows, appendrows(n = 1)->bool" },

    { "bind",
    (PyCFunction)ws_bindFunction,
    METH_VARARGS,
    "callbacks a function" },

    {"cursor",
    (PyCFunction)ws_GridCursor,
    METH_NOARGS,
    "returns the row and column coords of grid cursor, cursor()->int, int"},

    {"isOK",
    (PyCFunction)ws_isOK,
    METH_NOARGS,
    "return if Worksheet is still usable or not, isOK()->bool"},

    { "name",
    (PyCFunction)ws_name,
    METH_NOARGS,
    "returns the name of the worksheet" },

    { "ncols",
    (PyCFunction)ws_ncols,
    METH_NOARGS,
    "returns the number of columns, ncols()->int" },

    { "nrows",
    (PyCFunction)ws_nrows,
    METH_NOARGS,
    "returns the number of rows, nrows()->int" },

    { "selection",
    (PyCFunction)ws_selection,
    METH_NOARGS,
    "returns the selected area as Range object" },

    { "sel_coords",
    (PyCFunction)ws_selcoords,
    METH_NOARGS,
    "returns the selected area's coordinates (top-left, bottom-right)" },

    { "unbind",
    (PyCFunction)ws_UnbindFunction,
    METH_VARARGS,
    "unbinds the bound function" },

    {NULL}
};






/********************** Methods Used By type/mapping/sequence/nb **************************/

static int Worksheet_init(Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
	const wchar_t* Name = L"";
    int row = 1000, col = 50;
	PyObject *ActiveWSObj = Py_None;

	const char* kwlist[] = { "name","nrows", "ncols", "active", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|uiiO", const_cast<char**>(kwlist), &Name, &row, &col, &ActiveWSObj))
	{
		std::cout << "Could not parse" << std::endl;
		return -1;
	}

	
	if(!Py_IsNone(ActiveWSObj))
	{
		if(!glbWorkbook)
			return -1;

		auto ws =  (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
		self->ptrObj = ws;
		self->state = true;

		return 0;
	}


    bool Success = glbWorkbook->AddNewWorksheet(Name, row, col);
    if (Success)
    {
        auto ws = (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();

        self->ptrObj = ws;
        self->state = true;

        ws->RegisterPyWS(self);

        return 0;
    }

    return -1;
}




static void ws_dealloc(Python::Worksheet* self)
{
    self->state = false;
}



static PyObject* ws_mp_subscript(PyObject* self, PyObject* args)
{
    auto SelfWS = (Python::Worksheet*)self;
    CHECKSTATE(SelfWS, nullptr);

    int Row, Col;
    if (!PyArg_ParseTuple(args, "ii", &Row, &Col))
        return nullptr;

    CHECKNONNEGATIVE_RET(Row, "row position must be >=0");
    CHECKNONNEGATIVE_RET(Col, "col position must be >=0");

    wxString CellValue = SelfWS->ptrObj->GetCellValue(Row, Col);

    return Python::Object_FromString(CellValue.ToStdWstring());
}



static int ws_mp_ass_subscript(PyObject* self, PyObject* key, PyObject* rhs)
{
    auto SelfWS = (Python::Worksheet*)self;
    CHECKSTATE(SelfWS, -1);

    int Row, Col;
    if (!PyArg_ParseTuple(key, "ii", &Row, &Col))
        return -1;

    CHECKNONNEGATIVE_RETTYPE(Row, "row position must be >=0", -1);
    CHECKNONNEGATIVE_RETTYPE(Col, "col position must be >=0", -1);


    wxFont font = SelfWS->ptrObj->GetCellFont(Row, Col);

    if (IsSubTypeDict(rhs))
    {
        PyObject* Dict = rhs;

        PyObject* DictKey = nullptr, * DictValue = nullptr;
        Py_ssize_t pos = 0;

        wxString CellValue;
        wxColor CellBGColor, CellFGColor;

        while (PyDict_Next(Dict, &pos, &DictKey, &DictValue))
        {
            std::string KV = _PyUnicode_AsString(DictKey);
            if (KV == "value")
                SelfWS->ptrObj->SetCellValue(Row, Col, PyUnicode_AsWideCharString(DictValue, nullptr));

            else if (KV == "fgcolor" || KV == "fgcolour" || KV == "bgcolor" || KV == "bgcolour")
            {
				if((IsExactTypeString(DictValue) == false))
				{ 
					PyErr_SetString(PyExc_TypeError, "color values must be string type, \"R G B\""); 
					return -1; 
				}

                auto ColorStr = PyUnicode_AsUTF8(DictValue);
                const auto [R, G, B] = rgb(ColorStr);

                if (KV == "fgcolor" || KV == "fgcolour")
                    SelfWS->ptrObj->SetCellTextColour(Row, Col, wxColor(R, G, B));
                else
                    SelfWS->ptrObj->SetCellBackgroundColour(Row, Col, wxColor(R, G, B));
            }

            else if (KV == "style")
            {
                std::string StyleVal = PyUnicode_AsUTF8(DictValue);

                if (StyleVal == "italic")
                    font.MakeItalic();
                else if (StyleVal == "normal")
                    font.SetStyle(wxFONTSTYLE_NORMAL);
                ELSE_PYERR_RETDEF(PyExc_TypeError, "The value for the key style must be either \"italic\" or \"normal\"", -1);
            }

            else if (KV == "weight")
            {
                std::string WeightVal = PyUnicode_AsUTF8(DictValue);

                if (WeightVal == "bold")
                    font.MakeBold();
                else if (WeightVal == "normal")
                    font.SetWeight(wxFONTWEIGHT_NORMAL);
                ELSE_PYERR_RETDEF(PyExc_TypeError, "The value for the key weight must be either \"normal\" or \"bold\"", -1);
            }

            else if (KV == "underline")
            {
                std::string UnderLineVal = PyUnicode_AsUTF8(DictValue);

                if (UnderLineVal == "single")
                    font.SetUnderlined(true);
                else if (UnderLineVal == "none")
                    font.SetUnderlined(false);
                ELSE_PYERR_RETDEF(PyExc_TypeError, "The value for the key underline must be either \"single\" or \"none\"", -1);
            }

            SelfWS->ptrObj->SetCellFont(Row, Col, font);
            SelfWS->ptrObj->RefreshBlock(wxGridCellCoords(Row, Col), wxGridCellCoords(Row, Col));
        }
    }
    else
        SelfWS->ptrObj->SetCellValue(Row, Col, PyUnicode_AsWideCharString(PyObject_Str(rhs), nullptr));

    return 0;
}



static PyObject* ws_tp_str(PyObject* self)
{
    auto WS = (Python::Worksheet*)self;
    CHECKSTATE(WS, nullptr);

    wxString str = WS->ptrObj->GetWSName();

    return PyUnicode_FromString(str.mb_str(wxConvUTF8));
}



static PyMappingMethods ws_MappingMethods = { };


PyTypeObject PythonWorksheet_Type = { PyVarObject_HEAD_INIT(NULL, 0) "Worksheet" };



int PyInit_Worksheet(PyObject* Module)
{
    ws_MappingMethods.mp_subscript = ws_mp_subscript;
    ws_MappingMethods.mp_ass_subscript = ws_mp_ass_subscript;

    PythonWorksheet_Type.tp_new = PyType_GenericNew;
    PythonWorksheet_Type.tp_basicsize = sizeof(Python::Worksheet);

    PythonWorksheet_Type.tp_dealloc = (destructor)ws_dealloc;
    PythonWorksheet_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PythonWorksheet_Type.tp_doc = "Appends a worksheet to the workbook";

    PythonWorksheet_Type.tp_methods = PyWorksheet_methods;
    PythonWorksheet_Type.tp_init = (initproc)Worksheet_init;
    PythonWorksheet_Type.tp_str = (reprfunc)ws_tp_str;


    PythonWorksheet_Type.tp_as_mapping = &ws_MappingMethods;

    if (PyType_Ready(&PythonWorksheet_Type) < 0)
        return -1;


    Py_INCREF(&PythonWorksheet_Type);
    if (PyModule_AddObject(Module, "Worksheet", (PyObject*)&PythonWorksheet_Type) < 0)
    {
        Py_DECREF(&PythonWorksheet_Type);
        return -1;
    }

    return 0;
}