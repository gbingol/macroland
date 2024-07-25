#pragma once

#include "pkg_scisuit.h"

static struct PyMethodDef SYSTEMMethods[] = 
{
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef SYSTEMModule = {
    PyModuleDef_HEAD_INIT,
    "SCISUITSYSTEM",   /* name of module */
    "Do not directly use this module, it is used by the system", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SYSTEMMethods
};


static PyObject* CreateSystemModule()
{
    return PyModule_Create(&SYSTEMModule);
}

/*************************    GUI   *******************************/


static struct PyMethodDef scisuit_gui_methods[] =
{

    { "statbar_write",
    (PyCFunction)pkgscisuit::gui::statbar_write,
    METH_VARARGS | METH_KEYWORDS,
    "writes text to nth field, statbar_write(text=, n=)" },

	{ "messagebox",
    (PyCFunction)pkgscisuit::gui::messagebox,
    METH_VARARGS | METH_KEYWORDS,
    "display a message box" },

	{ "worksheetcount",
    (PyCFunction)pkgscisuit::workbook::numberofworksheets,
    METH_NOARGS,
    "returns the number of worksheets)" },

	{ "findworksheet",
    (PyCFunction)pkgscisuit::workbook::findworksheet,
    METH_VARARGS,
    "finds the worksheet with given name or page number, findworksheet(param)" },

	 { "Bind",
    (PyCFunction)pkgscisuit::workbook::BindFunction,
    METH_VARARGS,
    "callbacks a function when an event happens in Workbook (not worksheet)" },

    { "Unbind",
    (PyCFunction)pkgscisuit::workbook::UnbindFunction,
    METH_VARARGS,
    "unbinds the bound function from Workbook" },


   { NULL, NULL, 0, NULL }
};



static struct PyModuleDef System_GUI_Module = {
    PyModuleDef_HEAD_INIT,
    "SCISUITSYSTEM.GUI",
    "GUI sub module. Do not use this, instead use scisuit.stats",
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    scisuit_gui_methods
};


/*************************************************** */

static struct PyMethodDef CommandEditorMethods[] = { 
	{ NULL, NULL, 0, NULL } 
};

static struct PyModuleDef CommandEditorModuleDef = {
	PyModuleDef_HEAD_INIT,
	"SCISUITSYSTEM.CommandWindow",
	"Command Window Module",
	-1,
	CommandEditorMethods,
};
