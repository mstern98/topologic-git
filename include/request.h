#ifndef __TOPOLOGIC_REQUEST__
#define __TOPOLOGIC_REQUEST__

/**
Enum for submiting a request to be handles
MODIFY: Modify values in existing edges or vertices
        as well as add vertices or edges
DESTROY_VERTEX: Remove vertex from graph
DESTROY_EDGE: Remove edge from graph
**/
enum REQUESTS
{
        MODIFY = 0,
        DESTROY_VERTEX = 1,
        DESTROY_EDGE = 2
};

/** Request **/
struct request
{
        enum REQUESTS request;
        void (*f)(void *);
        int argc;
        void **args;
};

#endif