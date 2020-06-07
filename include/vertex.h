#ifndef __TOPOLOGIC_VERTEX__
#define __TOPOLOGIC_VERTEX__

#include <signal.h>
#include <stdio.h>
#include "./stack.h"
#include "./AVL.h"

/**vertex_result**/
struct vertex_result{
    void *vertex_argv;
    void *edge_argv;
};

/**vertex **/
struct vertex{
    int id; //Hash for number passed in, 
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    int is_active;
    struct vertex_result *(*f)(void *);
    int glblc; 
    void *glbl;
    int edge_sharedc;
    void *edge_shared; 
    pthread_mutex_t lock;
    struct AVLTree* edge_tree;
};

struct vertex_request{
    struct graph *graph;
    int id; //Hash for number passed in, 
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    struct vertex_result *(*f)(void *);
    int glblc; 
    void *glbl; 
};

struct mod_vertex_request{
    struct vertex *vertex;
    struct vertex_result *(*f)(void *);
    int glblc;
    void *glbl;
};

struct mod_edge_vars_request{
    struct vertex *vertex;
    int edgec;
    void *edge_vars;
};

struct destroy_vertex_request{
    struct graph *graph;
    struct vertex *vertex;
};

#endif
