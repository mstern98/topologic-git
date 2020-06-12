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

%include "../include/graph.h"
%extend graph {
    graph(int max_state_changes = -1,
        unsigned int snapshot_timestamp = START_STOP,
        unsigned int max_loop = MAX_LOOPS,
        enum VERBOSITY lvl_verbose = VERTICES | EDGES | FUNCTIONS | GLOBALS,
        enum CONTEXT context = SINGLE,
        enum MEM_OPTION mem_option = CONTINUE) {
            return graph_init(max_state_changes, snapshot_timestamp, max_loop, lvl_verbose, context, mem_option);
        }
    
    ~graph() {
        destroy_graph($self);
    }

    int set_start_set(int *id, int num_vertices) {
        return start_set($self, id, num_vertices);
    }

    int run(void *vertex_args[]) {
        return run($self, vertex_args);
    }
};

%include "../include/request.h"
%extend request {
    request(enum REQUESTS request, PyObject *args, void (*f)(PyObject *) = NULL) {
        return create_request(request, args, f);
    }
    ~request() {
        destroy_request($self);
    }
};

