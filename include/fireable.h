/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_FIREABLE__
#define __TOPOLOGIC_FIREABLE__

#ifdef __cplusplus
extern "C" {
#endif
 
struct fireable
{
        struct graph *graph;
        struct vertex *vertex;
        struct vertex_result *args;
        enum STATES color;
        int iloop;
};

#ifdef __cplusplus
}
#endif

#endif
