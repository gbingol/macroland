#pragma once

#include "PythonWrapper.h"
#include "guielements.h"


namespace pkgscisuit::gui
{
	PyObject* messagebox(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* statbar_gettext(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject *Enable(PyObject *self, PyObject *args);
}


namespace pkgscisuit::workbook
{

	PyObject* numberofworksheets(PyObject* self);

	PyObject *BindFunction(PyObject *self, PyObject *args);

	PyObject *UnbindFunction(PyObject *self, PyObject *args);
}


namespace pkgscisuit::extend
{
	PyObject* AppendToStatBarContextMenu(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* AppendToWorkbookContextMenu(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* AppendToWorkbook_Tab_ContextMenu(PyObject* self, PyObject* args, PyObject* kwargs);
	

	PyObject* AddToolBarPage(PyObject* self, PyObject* args, PyObject* kwargs);

	//This is needed as exec command does not work when embedded libs are imported
	PyObject *RunPythonFile(PyObject *self, PyObject *args);

	extension::CButton *MakeButton(PyObject *obj);

	extension::CHybridButton *MakeHybridButton(PyObject *obj);

	extension::CMenu *MakeMenu(PyObject *obj);

	extension::CToolBarPage* MakePage(PyObject* obj);

	void Menu_AddButton(wxMenu *Menu, extension::CButtonBase *btn);

	bool AddtoContextMenu(PyObject *Object, wxMenu *ContextMenu);
}

