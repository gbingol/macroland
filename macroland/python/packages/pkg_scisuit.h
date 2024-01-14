#pragma once

#include "../PythonWrapper.h"


namespace pkgscisuit::gui
{
	PyObject* activeworksheet(PyObject* self);

	PyObject* statbar_write(PyObject* self, PyObject* args, PyObject* kwargs);
}
