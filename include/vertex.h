#ifndef __TOPOLOGIC_VERTEX__
#define __TOPOLOGIC_VERTEX__

#include <signal.h>
#include <stdio.h>
#include "./stack.h"
#include "./AVL.h"

/**vertex **/
struct vertex{
    int id; //Hash for number passed in, 
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    void (*f)(void *);
    int argc;
    int glblc; 
    void *glbl;
    void *edge_shared; 
    sig_atomic_t active;
    struct AVLTree* edge_tree;
};

#endif