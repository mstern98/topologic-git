/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_VERTEX__
#define __TOPOLOGIC_VERTEX__

#include <signal.h>
#include <stdio.h>
#include "./stack.h"
#include "./AVL.h"
#include "./context.h"
#include "./graph.h"

#ifdef __cplusplus
extern "C" {
#endif

/**vertex_result**/
struct vertex_result
{
    void *vertex_argv;
    size_t vertex_size;
    void *edge_argv;
    size_t edge_size;
};

union shared_edge {
    void *vertex_data;
    const void *const *edge_data;
};

/**vertex **/
struct vertex
{
    int id; //Hash for number passed in,
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    int is_active;
    void (*f)(struct graph *, struct vertex_result *, void *, void *);
    void *glbl;
    union shared_edge *shared;
    pthread_mutex_t lock;
    struct AVLTree *edge_tree;
    struct AVLTree *joining_vertices;
    enum CONTEXT context;
};

struct vertex_request
{
    struct graph *graph;
    int id; //Hash for number passed in,
            //will compare with other vertices in graph
            //Must be unique. If non-unique ID, error
    void (*f)(struct graph *, struct vertex_result *, void *, void *);
    void *glbl;
};

struct mod_vertex_request
{
    struct vertex *vertex;
    void (*f)(struct graph *, struct vertex_result *, void *, void *);
    void *glbl;
};

struct mod_edge_vars_request
{
    struct vertex *vertex;
    void *edge_vars;
};

struct destroy_vertex_request
{
    struct graph *graph;
    struct vertex *vertex;
};

struct destroy_vertex_id_request
{
    struct graph *graph;
    int id;
};

#ifdef __cplusplus
}
#endif

#endif
