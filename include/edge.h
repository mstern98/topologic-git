#ifndef __TOPOLOGIC_EDGE__
#define __TOPOLOGIC_EDGE__

#include <stdio.h>
#include "./vertex.h"

/** Edge **/
struct edge{
    int id; //Hash for number passed in, 
            //will compare with other Edges in graph
            //Also unique, a la vertex 
            //Perhaps @nanosecond level
    void (*f)(void *);
    int argc;
    int glblc;
    void *glbl;
    int a_varc;
    void *a_vars; //To be shared among vertex a and shared edge
    struct vertex* a;
    struct vertex* b;
};

#endif