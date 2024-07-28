
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



static PyObject* ws_setcellcolor(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int row=-1, col=-1;
	PyObject *ColorObj{nullptr}, *TargetObj{nullptr};

	const char* kwlist[] = { "row", "col", "color", "target", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiOO", const_cast<char**>(kwlist), &row, &col, &ColorObj, &TargetObj))
        return nullptr;

    CHECKSTATE(self, nullptr);

	auto ColorStr = PyUnicode_AsUTF8(ColorObj);
	const auto [R, G, B] = rgb(ColorStr);

	std::string Target = PyUnicode_AsUTF8(TargetObj);
	if(Target == "bg")
		self->ptrObj->SetCellBackgroundColour(row, col, wxColor(R, G, B));
	else
		self->ptrObj->SetCellTextColour(row, col, wxColor(R, G, B));

	Py_RETURN_NONE;
}


static PyObject* ws_getcellcolor(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int row=-1, col=-1;
	PyObject *TargetObj{nullptr};

	const char* kwlist[] = { "row", "col", "target", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiO", const_cast<char**>(kwlist), &row, &col, &TargetObj))
        return nullptr;

    CHECKSTATE(self, nullptr);

	wxColor color;

	std::string Target = PyUnicode_AsUTF8(TargetObj);
	if(Target == "bg")
		color = self->ptrObj->GetCellBackgroundColour(row, col);
	else
		color = self->ptrObj->GetCellTextColour(row, col);

	PyObject *TupleObj = PyTuple_New(3);
	PyTuple_SetItem(TupleObj, 0, Py_BuildValue("i", color.GetRed()));
	PyTuple_SetItem(TupleObj, 1, Py_BuildValue("i", color.GetGreen()));
	PyTuple_SetItem(TupleObj, 2, Py_BuildValue("i", color.GetBlue()));

	return TupleObj;
}



static PyObject* ws_setcellfont(
    Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
    int row=-1, col=-1;
	PyObject *StyleObj{nullptr}, *WeightObj{nullptr}, *DecorationObj{nullptr};

	const char* kwlist[] = { "row", "col", "style", "weight", "decoration", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiOOO", const_cast<char**>(kwlist), 
				&row, &col, 
				&StyleObj, &WeightObj, &DecorationObj))
        return nullptr;

    CHECKSTATE(self, nullptr);

	 wxFont font = self->ptrObj->GetCellFont(row, col);

	if(!Py_IsNone(StyleObj))
	{
		if (Py_IsTrue(StyleObj))
			font.MakeItalic();
		else
			font.SetStyle(wxFONTSTYLE_NORMAL);
	}

	if(!Py_IsNone(WeightObj))
	{
		if (Py_IsTrue(WeightObj))
			font.MakeBold();
		else
			font.SetWeight(wxFONTWEIGHT_NORMAL);
	}

	if(!Py_IsNone(DecorationObj))
	{
		font.SetUnderlined(Py_IsTrue(DecorationObj));
	}

	self->ptrObj->SetCellFont(row, col, font);

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



static PyObject* ws_select(Python::Worksheet* self, PyObject* args)
{
    int TL_Row{}, TL_Col{}, BR_Row{}, BR_Col{};
     if (!PyArg_ParseTuple(args, "iiii", &TL_Row, &TL_Col, &BR_Row, &BR_Col))
        return nullptr;

    auto SelfWS = (Python::Worksheet*)self;
    CHECKSTATE(SelfWS, nullptr);

    SelfWS->ptrObj->SelectBlock(TL_Row, TL_Col, BR_Row, BR_Col);

    Py_RETURN_NONE;
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

	{ "setcellcolor",
    (PyCFunction)ws_setcellcolor,
    METH_VARARGS | METH_KEYWORDS,
    "sets the foreground or background color of a cell-> setvalue(row, col, color, target='fg')" },

	{ "getcellcolor",
    (PyCFunction)ws_getcellcolor,
    METH_VARARGS | METH_KEYWORDS,
    "gets the foreground or background color of a cell-> getvalue(row, col, target='fg')" },

	{ "setcellfont",
    (PyCFunction)ws_setcellfont,
    METH_VARARGS | METH_KEYWORDS,
    "sets the font (italic, bold, underlined) of a cell-> setvalue(row, col, style=None, weight=None, decoration=None)" },

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

    { "select",
    (PyCFunction)ws_select,
    METH_VARARGS,
    "selects the given coordinates (TL_Row, TL_Col, BR_Row, BR_Col)" },

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
    int row = -1, col = -1;
	PyObject *SearchObj = Py_None;

	const char* kwlist[] = { "name","nrows", "ncols", "search", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|uiiO", 
        const_cast<char**>(kwlist), 
        &Name, &row, &col, &SearchObj)) 
    {
		std::cout << "Could not parse" << std::endl;
		return -1;
	}

	if(!glbWorkbook)
		return -1;

	ICELL::CWorksheet *ws{nullptr};

	if(!Py_IsNone(SearchObj)) 
	{ 
		if(PyUnicode_Check(SearchObj))
		{
			std::wstring Text = PyUnicode_AsWideCharString(SearchObj, nullptr);
			ws = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(Text);
		}

		else if(PyLong_Check(SearchObj))
		{
			size_t PageNum = PyLong_AsLong(SearchObj);
			ws = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(PageNum);
		}

		if(!ws)
		{
			PyErr_SetString(PyExc_RuntimeError, "Worksheet could not be found."); 
			return -1; 
		}

	}
    else if(row>0 && col>0)
	{
        bool Success = glbWorkbook->AddNewWorksheet(Name, row, col);
        if(!Success)
		{ 
            PyErr_SetString(PyExc_RuntimeError, "Could not add a new worksheet!"); 
            return -1; 
        }
		ws =  (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
    }
	else if(row<=0 || col<=0)
		ws =  (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();


	if(!ws)
	{
		PyErr_SetString(PyExc_RuntimeError, "Unknown error!"); 
		return -1; 
	}

	self->ptrObj = ws;
    self->state = true;

    ws->RegisterPyWS(self);

    return 0;
}




static void ws_dealloc(Python::Worksheet* self)
{
    self->state = false;
}



static PyMappingMethods ws_MappingMethods = { };


PyTypeObject PythonWorksheet_Type = { PyVarObject_HEAD_INIT(NULL, 0) "Worksheet" };



int PyInit_Worksheet(PyObject* Module)
{
    PythonWorksheet_Type.tp_new = PyType_GenericNew;
    PythonWorksheet_Type.tp_basicsize = sizeof(Python::Worksheet);

    PythonWorksheet_Type.tp_dealloc = (destructor)ws_dealloc;
    PythonWorksheet_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PythonWorksheet_Type.tp_doc = "Appends a worksheet to the workbook";

    PythonWorksheet_Type.tp_methods = PyWorksheet_methods;
    PythonWorksheet_Type.tp_init = (initproc)Worksheet_init;

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