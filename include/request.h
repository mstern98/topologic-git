/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_REQUEST__
#define __TOPOLOGIC_REQUEST__

#ifdef __cplusplus
extern "C" {
#endif

/**
Enum for submiting a request to be handles
MODIFY: Modify values in existing edges or vertices
        as well as add vertices or edges
DESTROY_VERTEX: Remove vertex from graph
DESTROY_EDGE: Remove edge from graph
**/
enum REQUESTS
{
        CREAT_VERTEX = 0,
        CREAT_EDGE = 1,
        CREAT_BI_EDGE = 2,
        MOD_VERTEX = 3,
        MOD_EDGE_VARS = 4,
        MOD_EDGE = 5,
        MOD_BI_EDGE = 6,
        DESTROY_VERTEX = 7,
        DESTROY_VERTEX_BY_ID = 8,
        DESTROY_EDGE = 9,
        DESTROY_BI_EDGE = 10,
        DESTROY_EDGE_BY_ID = 11,
        GENERIC = 12
};

/** Request **/
struct request
{
        enum REQUESTS request;
        void (*f)(void *);
        void *args;
};

#ifdef __cplusplus
}
#endif

#endif
