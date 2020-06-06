#ifndef __TOPOLOGIC_VERTEX__
#define __TOPOLOGIC_VERTEX__

#include <signal.h>
#include <stdio.h>
#include "./stack.h"
#include "./AVL.h"

/**vertex_result**/
struct vertex_result{
    int vertex_argc;
    void *vertex_argv;
    int edge_argc;
    void *edge_argv;
};

/**vertex **/
struct vertex{
    int id; //Hash for number passed in, 
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    int is_active;
    struct vertex_result *(*f)(int, void *);
    int argc;
    int glblc; 
    void *glbl;
    int edge_sharedc;
    void *edge_shared; 
    pthread_mutex_t lock;
    struct AVLTree* edge_tree;
};

#endif
