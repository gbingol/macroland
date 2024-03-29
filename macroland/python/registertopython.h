#pragma once

#include "packages/pkg_scisuit.h"

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
    { "activeworksheet",
    (PyCFunction)pkgscisuit::gui::activeworksheet,
    METH_NOARGS,
    "returns the currently active worksheet, activeworksheet()->Worksheet" },

    { "statbar_write",
    (PyCFunction)pkgscisuit::gui::statbar_write,
    METH_VARARGS | METH_KEYWORDS,
    "writes text to nth field, statbar_write(text=, n=)" },


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
