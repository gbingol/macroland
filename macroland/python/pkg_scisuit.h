#pragma once

#include "PythonWrapper.h"
#include "../lua/lua_guielements.h"


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


namespace pkgscisuit::extension
{
	PyObject* AppendToStatBarContextMenu(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* AddToolBarPage(PyObject* self, PyObject* args, PyObject* kwargs);

	lua::CButton *MakeButton(PyObject *obj);

	lua::CHybridButton *MakeHybridButton(PyObject *obj);

	lua::CMenu *MakeMenu(PyObject *obj);

	lua::CToolBarPage* MakePage(PyObject* obj);

	void Menu_AddButton(wxMenu *Menu, lua::CButtonBase *btn);

	bool AddtoContextMenu(PyObject *Object, wxMenu *ContextMenu);
}

