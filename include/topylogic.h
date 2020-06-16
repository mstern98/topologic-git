#ifndef __TOPYLOGIC__
#define __TOPYLOGIC__

#include <Python.h>

struct topylogic_function 
{
    PyObject *f;
};

PyObject *callback(struct topylogic_function *tf, PyObject *args);

#endif