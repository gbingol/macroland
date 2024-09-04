#include "pkg_scisuit.h"

#include <filesystem>
#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/display.h>

#include "../cmdwnd/scripting_funcs.h"
#include "../cmdwnd/pnlcmdwnd.h"
#include "../mainfrm/frmmacroland.h"
#include "../mainfrm/icell.h"

#include"../macrolandapp.h"
#include "PythonWrapper.h"



extern std::filesystem::path glbExeDir;
extern ICELL::CWorkbook* glbWorkbook;



namespace pkgscisuit::gui
{

	PyObject *messagebox(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		PyObject* MsgObj{nullptr}, *CaptionObj{nullptr}, *YesNoObj{nullptr};

		const char* kwlist[] = { "msg", "caption", "yesno", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", 
				const_cast<char**>(kwlist), 
				&MsgObj, &CaptionObj, &YesNoObj))
			return nullptr;

		auto Msg = PyUnicode_AsWideCharString(MsgObj, nullptr);
		auto Caption = PyUnicode_AsWideCharString(CaptionObj, nullptr);

		bool IsYesNo = Py_IsTrue(YesNoObj);

		if(!IsYesNo)
			wxMessageBox(Msg, Caption);
		else {
			int ans = wxMessageBox(Msg, Caption, wxYES_NO);
			return Py_BuildValue("i", ans == wxYES ? 1 : 0);
		}

		Py_RETURN_NONE;
	}


	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		PyObject* TextObj = nullptr;
		int n = 0;

		const char* kwlist[] = { "text", "n", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi", const_cast<char**>(kwlist), &TextObj, &n))
			return nullptr;

		std::wstring Text = PyUnicode_AsWideCharString(TextObj, nullptr);

		IF_PYERRRUNTIME(!glbWorkbook, "No workbook found.", nullptr);
		glbWorkbook->SetStatusText(Text, n); 
		
		Py_RETURN_NONE;
	}


	PyObject * statbar_gettext(PyObject * self, PyObject * args, PyObject * kwargs)
	{
		int n;

		const char* kwlist[] = { "n", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", const_cast<char**>(kwlist), &n))
			return nullptr;

		auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
		if(frmSciSuit)
		{
			auto StatBar = frmSciSuit->GetStatusBar();
			if(StatBar)
				return PyUnicode_FromWideChar(StatBar->GetStatusText(n).ToStdWstring().c_str(),  -1);
		}

		Py_RETURN_NONE;
	}


	PyObject *Enable(PyObject *self, PyObject *args)
	{
		IF_PYERRRUNTIME(!glbWorkbook, "No workbook found.", nullptr);

		PyObject* EnableObj = PyTuple_GetItem(args, 0);
		glbWorkbook->Enable(Py_IsTrue(EnableObj));
		glbWorkbook->GetParent()->Enable(Py_IsTrue(EnableObj));

		Py_RETURN_NONE;
	}

}

/************************************************************************ */


namespace pkgscisuit::workbook
{

	PyObject *numberofworksheets(PyObject *self)
	{
		if(!glbWorkbook)
			return nullptr;

		return Py_BuildValue("i", glbWorkbook->GetWorksheetNotebook()->GetPageCount());
	}



	PyObject *BindFunction(PyObject *self, PyObject *args)
	{
		// Parameter checks are done from Python side
		if(!glbWorkbook)
			return nullptr;

		auto NameObj = PyTuple_GetItem(args, 0);
		auto Name = PyUnicode_AsUTF8(NameObj);

		auto Func = PyTuple_GetItem(args, 1);
		auto Args = PyTuple_GetItem(args, 2);
		Py_IncRef(Args);

		auto Callback = new Python::CEventCallbackFunc();
		Callback->m_Func = Func;
		Callback->m_Args = Args;
		glbWorkbook->BindPythonFunction(Name, Callback);

		Py_RETURN_NONE;
	}


	PyObject *UnbindFunction(PyObject *self, PyObject *args)
	{
		//Parameter checks are done from Python side
		if(!glbWorkbook)
			return nullptr;

		auto EventNameObj = PyTuple_GetItem(args, 0);

		std::string EventName = PyUnicode_AsUTF8(EventNameObj);
		auto FuncObj = PyTuple_GetItem(args, 1);
		glbWorkbook->UnbindPythonFunction(EventName, FuncObj);
		

    	Py_RETURN_NONE;
	}


} //namespace




/************************************ Worksheet Python Class*************************************** */

#define CHECKSTATE(OBJ, RET)					                                        \
if ((OBJ)->state == false) {                                                             \
    PyErr_SetString(PyExc_ReferenceError, "Object is not usable anymore");   \
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
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", 
		const_cast<char**>(kwlist), &row, &col))
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
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiO", 
		const_cast<char**>(kwlist), &row, &col, &ValueObj))
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
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiOO", 
		const_cast<char**>(kwlist), &row, &col, &ColorObj, &TargetObj))
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
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iiO", 
		const_cast<char**>(kwlist), &row, &col, &TargetObj))
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

	if(!Py_IsNone(StyleObj)) {
		if (Py_IsTrue(StyleObj))
			font.MakeItalic();
		else
			font.SetStyle(wxFONTSTYLE_NORMAL);
	}

	if(!Py_IsNone(WeightObj)) {
		if (Py_IsTrue(WeightObj))
			font.MakeBold();
		else
			font.SetWeight(wxFONTWEIGHT_NORMAL);
	}

	if(!Py_IsNone(DecorationObj))
		font.SetUnderlined(Py_IsTrue(DecorationObj));

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

	/*
		Types are checked from Python side
		def bind(self, event:str, func:_types.FunctionType, *args)->None:
	*/
    auto EventNameObj = PyTuple_GetItem(args, 0);
    auto Func = PyTuple_GetItem(args, 1);
	auto Args = PyTuple_GetItem(args, 2); //Tuple
	Py_IncRef(Args);

    auto CallBk = new Python::CEventCallbackFunc();
    CallBk->m_Func = Func;
    CallBk->m_Args = Args;
    SelfWS->ptrObj->BindPythonFunction(PyUnicode_AsUTF8(EventNameObj), CallBk);

    Py_RETURN_NONE;
}



static PyObject* ws_UnbindFunction(
    Python::Worksheet* self, PyObject* args)
{
    auto SelfWS = (Python::Worksheet*)self;
    CHECKSTATE(SelfWS, nullptr);

	//Types and number of args are checked from Python side
    auto NameObj = PyTuple_GetItem(args, 0);
    auto Func = PyTuple_GetItem(args, 1);

	SelfWS->ptrObj->UnbindPythonFunction(PyUnicode_AsUTF8(NameObj), Func);

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


static PyObject* ws_screencoord(Python::Worksheet* self, PyObject* args)
{
    CHECKSTATE(self, nullptr);

	PyObject* RowObj = PyTuple_GetItem(args, 0);
    PyObject* ColObj = PyTuple_GetItem(args, 1);
	PyObject* TLObj = PyTuple_GetItem(args, 2);

	auto ws = self->ptrObj;

	auto Coords = self->ptrObj->GetGridCursorCoords();
	int row = (int)PyLong_AsLong(RowObj);
	int col = (int)PyLong_AsLong(ColObj);
	bool IsTopLeft = Py_IsTrue(TLObj);

	if(!ws->IsVisible(row, col))
		Py_RETURN_NONE;

	auto GridWnd = ws->CellToGridWindow(row, col);
	auto cellRect = ws->CellToRect(row, col);
	cellRect.SetPosition(cellRect.GetPosition() + wxPoint(ws->GetRowLabelSize(), ws->GetColLabelSize()));
	auto TL = IsTopLeft ? cellRect.GetTopLeft() : cellRect.GetBottomRight();
	TL = ws->CalcGridWindowScrolledPosition(TL, GridWnd);
	TL = ws->ClientToScreen(TL);

	wxDisplay display(wxDisplay::GetFromWindow(glbWorkbook->GetParent()));
	auto screen = display.GetClientArea();

	if(TL.x>screen.GetWidth() || TL.y>screen.GetHeight())
		Py_RETURN_NONE;

	auto Tuple = PyTuple_New(2);
    PyTuple_SetItem(Tuple, 0, Py_BuildValue("i", TL.x));
    PyTuple_SetItem(Tuple, 1, Py_BuildValue("i", TL.y));

    return Tuple;
}



static PyObject* ws_isOK(Python::Worksheet* self)
{
    if (self->state) {
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
    return Py_BuildValue("i", self->ptrObj->GetNumberCols());
}


static PyObject* ws_nrows(Python::Worksheet* self)
{
    CHECKSTATE(self, nullptr);
    return Py_BuildValue("i", self->ptrObj->GetNumberRows());
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

    auto TL = self->ptrObj->GetSelTopLeft();
    auto BR = self->ptrObj->GetSelBtmRight();
    
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

	 { "screen_coords",
    (PyCFunction)ws_screencoord,
    METH_VARARGS,
    "returns the cell's screen coordinates (top-left, bottom-right)" },

    { "unbind",
    (PyCFunction)ws_UnbindFunction,
    METH_VARARGS,
    "unbinds the bound function" },

    {NULL}
};




static int Worksheet_init(Python::Worksheet* self, PyObject* args, PyObject* kwargs)
{
	IF_PYERRRUNTIME(!glbWorkbook, "Worbook not ready.", -1); 

	const wchar_t* Name = L"";
    int row = -1, col = -1;
	PyObject *SearchObj = Py_None;

	const char* kwlist[] = { "name","nrows", "ncols", "search", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|uiiO", 
        const_cast<char**>(kwlist), 
        &Name, &row, &col, &SearchObj)) 
		return -1;


	ICELL::CWorksheet *ws{nullptr};

	if(!Py_IsNone(SearchObj)) //search a worksheet
	{ 
		if(PyUnicode_Check(SearchObj)) {
			std::wstring Text = PyUnicode_AsWideCharString(SearchObj, nullptr);
			ws = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(Text);
		}

		else if(PyLong_Check(SearchObj)) {
			size_t PageNum = PyLong_AsLong(SearchObj);
			ws = (ICELL::CWorksheet*)glbWorkbook->GetWorksheet(PageNum);
		}

		IF_PYERRRUNTIME(!ws, "Worksheet could not be found.", -1); 
	}
    else if(row>0 && col>0) //add a new worksheet
	{
        bool Success = glbWorkbook->AddNewWorksheet(Name, row, col);
		IF_PYERRRUNTIME(!Success, "Could not add a new worksheet!", -1); 

		ws =  (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
    }
	else if(row<=0 || col<=0) //get active worksheet
		ws =  (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();

	IF_PYERRRUNTIME(!ws, "Unknown error!", -1);

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



namespace pkgscisuit::extend
{
	PyObject *AppendToStatBarContextMenu(PyObject *self, PyObject *args, PyObject *kwargs)
	{
		PyObject *ButtonObj{nullptr}, *FieldObj{nullptr};

		const char* kwlist[] = { "field", "button", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O", 
				const_cast<char**>(kwlist), 
				&FieldObj, &ButtonObj))
			return nullptr;
		
		auto frmSciSuit = (frmMacroLand*)wxTheApp->GetTopWindow();
		auto ContextMenu = frmSciSuit->getStatBarMenu();
		
		int Field = PyLong_AsLong(FieldObj) - 1;
		if (Field == frmSciSuit->getStBarRectField())
			AddtoContextMenu(ButtonObj, ContextMenu);
		
		Py_RETURN_NONE;
	}


	PyObject *extend::AppendToWorkbookContextMenu(PyObject *self, PyObject *args, PyObject *kwargs)
	{
		PyObject *Obj{nullptr};

		const char* kwlist[] = {"object", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", 
				const_cast<char**>(kwlist), &Obj))
			return nullptr;

		auto ActiveWS = (ICELL::CWorksheet*)glbWorkbook->GetActiveWS();
		auto ContextMenu = ActiveWS->GetContextMenu();
		AddtoContextMenu(Obj, ContextMenu);

		Py_RETURN_NONE;
	}


	PyObject* AppendToWorkbook_Tab_ContextMenu(PyObject *self, PyObject *args, PyObject *kwargs)
	{
		PyObject *Obj{nullptr};

		const char* kwlist[] = {"object", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", 
				const_cast<char**>(kwlist), &Obj))
			return nullptr;

		auto ContextMenu = glbWorkbook->GetWorksheetNotebook()->GetContextMenu();
		AddtoContextMenu(Obj, ContextMenu);

		Py_RETURN_NONE;
	}


	PyObject * AddToolBarPage(PyObject * self, PyObject * args, PyObject * kwargs)
	{
		PyObject *PageObj{nullptr};

		const char* kwlist[] = { "page", NULL };
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", 
				const_cast<char**>(kwlist), 
				&PageObj))
			return nullptr;

		if(!glbWorkbook)
			Py_RETURN_NONE;


		auto Ntbk = glbWorkbook->GetToolBarNtbk();
		
		auto Page = MakePage(PageObj);
		Ntbk->AddPage(Page);

		Py_RETURN_NONE;
	}



	extension::CButton *MakeButton(PyObject *obj)
	{
		if(!PyDict_Check(obj))
			return nullptr;
		
		auto TypeObj = PyDict_GetItemString(obj, "type");
		std::string Type = PyUnicode_AsUTF8(TypeObj);

		auto TitleObj = PyDict_GetItemString(obj, "title");
		auto Title = PyUnicode_AsWideCharString(TitleObj, nullptr);

		auto ImgObj = PyDict_GetItemString(obj, "img");
		auto Img = PyUnicode_AsWideCharString(ImgObj, nullptr);

		auto ModulePathObj = PyDict_GetItemString(obj, "module");
		auto ModulePath = PyUnicode_AsWideCharString(ModulePathObj, nullptr);

		auto FuncObj = PyDict_GetItemString(obj, "click");
		auto FuncName = PyUnicode_AsWideCharString(FuncObj, nullptr);

		//Tuple object
		auto ArgsObj = PyDict_GetItemString(obj, "args");

		auto btn = new extension::CButton(Title);
		btn->SetImgPath(Img);
		btn->SetModulePath(ModulePath);
		btn->SetFuncName(FuncName);
		btn->SetParam(ArgsObj);

		return btn;
	}



	extension::CHybridButton *MakeHybridButton(PyObject *obj)
	{
		auto TypeObj = PyDict_GetItemString(obj, "type");
		std::string Type = PyUnicode_AsUTF8(TypeObj);

		auto DictMainBtn = PyDict_GetItemString(obj, "mainbutton");
		auto MainButton = MakeButton(DictMainBtn);

		auto HybridBtn = new extension::CHybridButton(MainButton);

		auto List = PyDict_GetItemString(obj, "list");
		auto N = PyList_Size(List);
		for (size_t i = 0; i<N; ++i)
		{
			auto Item = PyList_GetItem(List, i);
			auto Btn = MakeButton(Item);
			HybridBtn->AddButton(Btn);
		}

		return HybridBtn;
	}



	extension::CMenu *MakeMenu(PyObject *obj)
	{
		auto TypeObj = PyDict_GetItemString(obj, "type");
		std::string Type = PyUnicode_AsUTF8(TypeObj);

		auto TitleObj = PyDict_GetItemString(obj, "title");
		auto Title = PyUnicode_AsWideCharString(TitleObj, nullptr);

		auto ImgObj = PyDict_GetItemString(obj, "img");
		auto Img = PyUnicode_AsWideCharString(ImgObj, nullptr);

		auto Menu = new extension::CMenu(Title);
		Menu->SetImgPath(Img);

		auto List = PyDict_GetItemString(obj, "list");
		auto N = PyList_Size(List);
		for (size_t i = 0; i<N; ++i)
		{
			auto Item = PyList_GetItem(List, i);
			auto Btn = MakeButton(Item);
			Menu->AddButton(Btn);
		}

		return Menu;
	}


	extension::CToolBarPage * MakePage(PyObject * obj)
	{
		auto TypeObj = PyDict_GetItemString(obj, "type");
		std::string Type = PyUnicode_AsUTF8(TypeObj);

		auto TitleObj = PyDict_GetItemString(obj, "title");
		auto Title = PyUnicode_AsWideCharString(TitleObj, nullptr);

		auto Ntbk = glbWorkbook->GetToolBarNtbk();
		auto Page = Ntbk->FindPage(Title);

		extension::CToolBarPage* page{nullptr};
		if(!Page)
			page = new extension::CToolBarPage(Ntbk, Title);
		else
			page = (extension::CToolBarPage *)Page;
		
		auto List = PyDict_GetItemString(obj, "list");
		auto N = PyList_Size(List);
		for (size_t i = 0; i<N; ++i)
		{
			auto Item = PyList_GetItem(List, i);
			auto ItemTypeObj = PyDict_GetItemString(Item, "type");
			std::string ItemType = PyUnicode_AsUTF8(ItemTypeObj);

			extension::CElement* elem{nullptr};
			if(ItemType == "button")
				elem = MakeButton(Item);
			else if (ItemType == "hybridbutton")
				elem = MakeHybridButton(Item);

			page->AddElement(elem);
		}

		return page;
	}


	void Menu_AddButton(wxMenu* Menu, extension::CButtonBase* btn)
	{
		int btnID = btn->GetId();
		wxString Title = btn->GetTitle();
		wxBitmap bmp = btn->GetBitmap(btn->GetImagePath());

		auto Item = Menu->Append(btnID, Title);
		Item->SetBitmap(bmp);

		Menu->Bind(wxEVT_MENU, &extension::CButtonBase::OnClick, (extension::CButtonBase*)btn, btnID);
	}


	bool AddtoContextMenu(PyObject* Obj, wxMenu* ContextMenu)
	{
		if (Py_IsNone(Obj)) 
		{
			ContextMenu->AppendSeparator();
			return true;
		}

		try {
			auto TypeObj = PyDict_GetItemString(Obj, "type");
			std::string Type = PyUnicode_AsUTF8(TypeObj);

			if (Type == "button")
			{
				auto btn = MakeButton(Obj);
				if (btn->IsOK())
					Menu_AddButton(ContextMenu, btn);
			}
			else if (Type == "menu")
			{
				auto menu = MakeMenu(Obj);
				wxMenu* SubMenu = new wxMenu();

				for (auto btn : menu->GetList())
				{
					if (btn->IsOK())
						Menu_AddButton(SubMenu, btn);
				}

				auto MenuItem = ContextMenu->AppendSubMenu(SubMenu, menu->GetTitle());
				MenuItem->SetBitmap(menu->GetBitmap(menu->GetImagePath()));
			}
		}
		catch (std::exception& e)
		{ 
			PyErr_SetString(PyExc_RuntimeError, e.what()); 
			return false; 
		}

		return true;
	}

	PyObject * RunPythonFile(PyObject * self, PyObject * args)
	{
		IF_PYERRRUNTIME(!glbWorkbook, "No workbook found.", nullptr);

		auto PathObj = PyTuple_GetItem(args, 0);
		std::wstring Path = PyUnicode_AsWideCharString(PathObj, nullptr);
		auto Pth = PyUnicode_AsUTF8(PathObj);

		IF_PYERRRUNTIME(!std::filesystem::exists(Path), "Python file does not exist.", nullptr);

		if (auto cp = _Py_wfopen(Path.c_str(), L"rb"))  
			PyRun_SimpleFileExFlags(cp, Pth, true, 0);
		
		Py_RETURN_NONE;
	}

}