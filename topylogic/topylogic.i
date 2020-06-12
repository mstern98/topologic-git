%module topylogic
%{
#include "../include/topologic.h"
%}

%include "../include/stack.h"
%extend stack{
    stack() {
        return init_stack();
    }

    ~stack() {
        destroy_stack($self);
    }

    PyObject *get(int index) {
        PyObject *ret = get($self, index);
        if (!ret) return Py_None;
        return ret;
    }

    PyObject *pop() {
        PyObject *ret = pop($self);
        if (!ret) return Py_None;
        return ret;
    }

    int push(PyObject *data) {
        return push($self, data);
    }
};

%include "../include/AVL.h"
%extend AVLTree {
    AVLTree() {
        return init_avl();
    }
    ~AVLTree() {
        destroy_avl($self);
    }
    int insert(PyObject *data, int id) {
        return insert($self, data, id);
    }

    PyObject *remove_ID(int id) {
        PyObject *ret = remove_ID($self, id);
        if (!ret) return Py_None;
        return ret;
    }

    PyObject *find(int id) {
        PyObject *ret = find($self, id);
        if (!ret) return Py_None;
        return ret;
    }

    void inorder(struct stack *stack) {
        return inorder($self, stack);
    }

    void preorder(struct stack *stack){
        return preorder($self, stack);
    }

    void postorder(struct stack *stack){
        return postorder($self, stack);
    }

    void stackify(struct stack *stack){
        return stackify($self, stack);
    }
};

%include "../include/topologic.h"
