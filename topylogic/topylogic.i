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
%include "../include/vertex.h"
%include "../include/edge.h"
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

    %typemap (in) void** (void *temp)
    {
        if ((SWIG_ConvertPtr($input, (void **) &temp, $*1_descriptor, SWIG_POINTER_DISOWN)) == -1)
            temp = NULL;
        $1 = &temp;
    }
    int run(struct vertex_result *vertex_args[]) {
        return run($self, vertex_args);
    }

    int set_start_set(int *id, int num_vertices) {
        return start_set($self, id, num_vertices);
    }

    int pause_graph() {
        return pause_graph($self);
    }

    int resume_graph() {
        return resume_graph($self);
    }

    void print_graph() {
        return print_graph($self);
    }

    struct vertex *create_vertex(struct vertex_result *(*f)(void *), int id, PyObject *glbl = NULL) {
        return create_vertex($self, f, id, glbl);
    }

    int remove_vertex(struct vertex *vertex) {
        return remove_vertex($self, vertex);
    }

    int modify_vertex(struct vertex *vertex, struct vertex_result *(*f)(void *) = NULL, PyObject *glbl = NULL) {
        return modify_vertex(vertex, f, glbl);
    }

    int modify_shared_edge_vars(struct vertex *vertex, PyObject *edge_vars) {
        return modify_shared_edge_vars(vertex, edge_vars);
    }
 
    struct edge *create_edge(struct vertex *a, struct vertex *b, int (*f)(void *), PyObject *glbl = NULL) {
        return create_edge(a, b, f, glbl);
    }
    int create_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *), PyObject *glbl, struct edge **edge_a_to_b = NULL, struct edge **edge_b_to_a = NULL) {
        return create_bi_edge(a, b, f, glbl, edge_a_to_b, edge_b_to_a);
    }

    int remove_edge(struct vertex *a, struct vertex *b) {
        return remove_edge(a, b);
    }

    int remove_edge_id(struct vertex *a, int id) {
        return remove_edge_id(a, id);
    }

    int remove_bi_edge(struct vertex *a, struct vertex *b) {
        return remove_bi_edge(a, b);
    }

    int modify_edge(struct vertex *a, struct vertex *b, int (*f)(void *) = NULL, PyObject *glbl = NULL) {
        return modify_edge(a, b, f, glbl);
    }

    int modify_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *) = NULL, PyObject *glbl = NULL) {
        return modify_bi_edge(a, b, f, glbl);
    }

    struct request *create_request(enum REQUESTS request, PyObject *args, void (*f)(void *) = NULL) {
        return create_request(request, args, f);
    }

    int submit_request(struct request *request) {
        return submit_request($self, request);
    }

    int process_requests() {
        return process_requests($self);
    }
};

%extend vertex_result {
    vertex_result(PyObject *vertex_argv = NULL, PyObject *edge_argv = NULL) {
        struct vertex_result *v = malloc(sizeof(struct vertex_result));
        v->vertex_argv = vertex_argv;
        v->edge_argv = edge_argv;
        return v;
    }
    ~vertex_result() {
        free($self);
    }

    void set_vertex_args(PyObject *vertex_argv) {
        $self->vertex_argv = vertex_argv;
    }

    void set_edge_args(PyObject *edge_argv) {
        $self->edge_argv = edge_argv;
    }

    PyObject *get_vertex_args() {
        return $self->vertex_argv;
    }

    PyObject *get_edge_args() {
        return $self->edge_argv;
    }
}

%extend vertex_request {
    vertex_request(struct graph *graph, int id, struct vertex_result *(*f)(void *)=NULL, PyObject *glbl=NULL) {
        struct vertex_request *v = malloc(sizeof(struct vertex_request));
        v->graph = graph;
        v->id = id;
        v->f = f;
        v->glbl = glbl;
        return v;
    }
    ~vertex_request() {
        free($self);
    }
};

%extend mod_vertex_request {
    mod_vertex_request(struct vertex *vertex, struct vertex_result *(*f)(void *)=NULL, PyObject *glbl=NULL) {
        struct mod_vertex_request *v = malloc(sizeof(struct mod_vertex_request));
        v->vertex = vertex;
        v->f = f;
        v->glbl = glbl;
        return v;   
    }
    ~mod_vertex_request() {
        free($self);
    }
};

%extend mod_edge_vars_request {
    mod_edge_vars_request(struct vertex *vertex, PyObject *edge_vars=NULL) {
        struct mod_edge_vars_request *v = malloc(sizeof(struct mod_edge_vars_request));
        v->vertex = vertex;
        v->edge_vars = edge_vars;
        return v;   
    }
    ~mod_edge_vars_request() {
        free($self);
    }
};

%extend destroy_vertex_request {
    destroy_vertex_request(struct graph *graph, struct vertex *vertex) {
        struct destroy_vertex_request *v = malloc(sizeof(struct destroy_vertex_request));
        v->vertex = vertex;
        v->graph = graph;
        return v;   
    }
    ~destroy_vertex_request() {
        free($self);
    }
};

%extend edge_request {
    edge_request(struct vertex *a, struct vertex *b, int (*f)(void *)=NULL, PyObject *glbl=NULL) {
        struct edge_request *e = malloc(sizeof(struct edge_request));
        e->a = a;
        e->b = b;
        e->f = f;
        e->glbl = glbl;
        return e;
    }
    ~edge_request() {
        free($self);
    }
};

%extend destroy_edge_request {
    destroy_edge_request(struct vertex *a, struct vertex *b) {
        struct destroy_edge_request *e = malloc(sizeof(struct destroy_edge_request));
        e->a = a;
        e->b = b;
        return e;
    }
    ~destroy_edge_request() {
        free($self);
    }
};

%extend destroy_edge_id_request {
    destroy_edge_id_request(struct vertex *a, int id) {
        struct destroy_edge_id_request *e = malloc(sizeof(struct destroy_edge_id_request));
        e->a = a;
        e->id = id;
        return e;
    }
    ~destroy_edge_id_request() {
        free($self);
    }
};
