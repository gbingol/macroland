#pragma once

#include "PythonWrapper.h"
#include "guielements.h"


namespace pkgscisuit::gui
{
	PyObject* messagebox(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject *Enable(PyObject *self, PyObject *args);
}


namespace pkgscisuit::workbook
{

	PyObject* numberofworksheets(PyObject* self);

	PyObject *BindFunction(PyObject *self, PyObject *args);

	PyObject *UnbindFunction(PyObject *self, PyObject *args);
}


namespace extension
{
	PyObject* ws_stbar_menu(PyObject* self, PyObject* args, PyObject* kwargs);

	CButton *MakeButton(PyObject *obj);

	CMenu *MakeMenu(PyObject *obj);

	void Menu_AddButton(wxMenu *Menu, CButtonBase *btn);

	PyObject* contextmenu(PyObject *Object, wxMenu *ContextMenu);
}

