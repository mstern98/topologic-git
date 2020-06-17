#ifndef __TOPYLOGIC__
#define __TOPYLOGIC__

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

struct topylogic_function 
{
    PyObject *f;
};

PyObject *callback(struct topylogic_function *tf, PyObject *args);

#ifdef __cplusplus
}
#endif

#endif