#pragma once

#include "pkg_scisuit.h"

static struct PyMethodDef SYSTEMMethods[] = 
{
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef SYSTEMModule = {
    PyModuleDef_HEAD_INIT,
    "__SCISUIT",   /* name of module */
    "System owned module", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SYSTEMMethods
};


PyMODINIT_FUNC CreateSystemModule()
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

	{ "enable",
    (PyCFunction)pkgscisuit::gui::Enable,
    METH_VARARGS,
    "enable/disable framework, Enable(active=True)" },

	{ "worksheetcount",
    (PyCFunction)pkgscisuit::workbook::numberofworksheets,
    METH_NOARGS,
    "returns the number of worksheets)" },


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
    "GUI",
    "GUI sub module.",
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
	"COMMANDWINDOW",
	"Command Window Module",
	-1,
	CommandEditorMethods,
};



/***************************************************************** */


static struct PyMethodDef scisuit_extension_methods[] =
{

    { "addtoolbarpage",
    (PyCFunction)pkgscisuit::extend::AddToolBarPage,
    METH_VARARGS | METH_KEYWORDS,
    "Adds a given page to toolbar" },

	{ "statbar_contextmenu_append",
    (PyCFunction)pkgscisuit::extend::AppendToStatBarContextMenu,
    METH_VARARGS | METH_KEYWORDS,
    "Appends menu items to status bar's context menu" },

	{ "workbook_contextmenu_append",
    (PyCFunction)pkgscisuit::extend::AppendToWorkbookContextMenu,
    METH_VARARGS | METH_KEYWORDS,
    "Appends menu items to workbook's context menu" },

    { "runpythonfile",
    (PyCFunction)pkgscisuit::extend::RunPythonFile,
    METH_VARARGS,
    "Runs a Python file" },

   { NULL, NULL, 0, NULL }
};



static struct PyModuleDef System_Extension_Module = {
    PyModuleDef_HEAD_INIT,
    "EXTENSION",
    "EXTENSION sub module.",
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    scisuit_extension_methods
};
