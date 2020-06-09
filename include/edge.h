#ifndef __TOPOLOGIC_EDGE__
#define __TOPOLOGIC_EDGE__

#include <stdio.h>
#include "./vertex.h"

enum edge_type{
    EDGE = 0,
    BI_EDGE = 1
};

/** Edge **/
struct edge{
    int id; //Hash for number passed in, 
            //will compare with other Edges in graph
            //Also unique, a la vertex 
            //Perhaps @nanosecond level
    int (*f)(void *);
    void *glbl;
    const void *const *a_vars; //To be shared among vertex a and shared edge
    struct vertex* a;
    struct vertex* b;
    enum edge_type edge_type;
    pthread_mutex_t bi_edge_lock;
};

struct edge_request{
    struct vertex *a;
    struct vertex *b;
    int (*f)(void *);
    void *glbl;
};

struct destroy_edge_request{
    struct vertex *a;
    struct vertex *b;
};

struct destroy_edge_id_request{
    struct vertex *a;
    int id;
};

#endif
