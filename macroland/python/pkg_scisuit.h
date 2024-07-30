#pragma once

#include "PythonWrapper.h"


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
