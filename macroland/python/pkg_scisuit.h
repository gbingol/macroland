#pragma once

#include "PythonWrapper.h"


namespace pkgscisuit::gui
{
	PyObject* messagebox(PyObject* self, PyObject* args, PyObject* kwargs);

	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs);


	PyObject* findworksheet(PyObject* self, PyObject* args);

	PyObject *BindFunction(PyObject *self, PyObject *args);

	PyObject *UnbindFunction(PyObject *self, PyObject *args);
}
