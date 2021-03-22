/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_EDGE__
#define __TOPOLOGIC_EDGE__

#include <stdio.h>
#include "./vertex.h"

#ifdef __cplusplus
extern "C" {
#endif

enum edge_type
{
    EDGE = 0,
    BI_EDGE = 1,
	SELF_EDGE = 2
};

/** Edge **/
struct edge
{
    int id; //Hash for number passed in,
            //will compare with other Edges in graph
            //Also unique, a la vertex
            //Perhaps @nanosecond level
    int (*f)(int, void *, void *, const void *const a_vars, const void *const b_vars);
    void *glbl;
    const void *const *a_vars; //To be shared among vertex a and shared edge
    const void *const *b_vars; //To be shared among vertex b. Will be NULL for context=SWITCH
    struct vertex *a;
    struct vertex *b;
    enum edge_type edge_type;
    struct edge *bi_edge;
    pthread_mutex_t bi_edge_lock;
};

struct edge_request
{
    struct vertex *a;
    struct vertex *b;
    int (*f)(int, void *, void *, const void *const, const void *const);
    void *glbl;
};

struct destroy_edge_request
{
    struct vertex *a;
    struct vertex *b;
};

struct destroy_edge_id_request
{
    struct vertex *a;
    int id;
};

#ifdef __cplusplus
}
#endif

#endif
