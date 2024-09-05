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

	{ "messagebox",
    (PyCFunction)pypkg::framework::messagebox,
    METH_VARARGS | METH_KEYWORDS,
    "display a message box" },

	{ "enable",
    (PyCFunction)pypkg::framework::Enable,
    METH_VARARGS,
    "enable/disable framework, Enable(active=True)" },

	{ "worksheetcount",
    (PyCFunction)pypkg::framework::numberofworksheets,
    METH_NOARGS,
    "returns the number of worksheets)" },


	 { "Bind",
    (PyCFunction)pypkg::framework::BindFunction,
    METH_VARARGS,
    "callbacks a function when an event happens in Workbook (not worksheet)" },

    { "Unbind",
    (PyCFunction)pypkg::framework::UnbindFunction,
    METH_VARARGS,
    "unbinds the bound function from Workbook" },

	{ "statbar_write",
    (PyCFunction)pypkg::framework::statbar_write,
    METH_VARARGS | METH_KEYWORDS,
    "writes text to nth field, statbar_write(text=, n=)" },

    { "statbar_gettext",
    (PyCFunction)pypkg::framework::statbar_gettext,
    METH_VARARGS | METH_KEYWORDS,
    "gets the text of nth field" },

	{ "statbar_Bind",
    (PyCFunction)pypkg::framework::statbar_BindFunction,
    METH_VARARGS,
    "callbacks a function when an event happens in status bar" },

    { "statbar_Unbind",
    (PyCFunction)pypkg::framework::statbar_UnbindFunction,
    METH_VARARGS,
    "unbinds the bound function from status bar" },


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
    (PyCFunction)pypkg::extend::AddToolBarPage,
    METH_VARARGS | METH_KEYWORDS,
    "Adds a given page to toolbar" },

	{ "statbar_contextmenu_append",
    (PyCFunction)pypkg::extend::AppendToStatBarContextMenu,
    METH_VARARGS | METH_KEYWORDS,
    "Appends menu items to status bar's context menu" },

	{ "workbook_contextmenu_append",
    (PyCFunction)pypkg::extend::AppendToWorkbookContextMenu,
    METH_VARARGS | METH_KEYWORDS,
    "Appends menu or button to workbook's context menu" },

	{ "workbook_tab_contextmenu_append",
    (PyCFunction)pypkg::extend::AppendToWorkbook_Tab_ContextMenu,
    METH_VARARGS | METH_KEYWORDS,
    "Appends menu or button to workbook's context menu" },

    { "runpythonfile",
    (PyCFunction)pypkg::extend::RunPythonFile,
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
