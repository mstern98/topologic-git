/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_CONSTANT__
#define __TOPOLOGIC_CONSTANT__

#ifdef __cplusplus
extern "C" {
#endif

/** 
Enum for how the graph handles context switches,
or not at all
NONE: First valid edge is taken only
      and the process does not change
SINGLE: Is the same as NONE 
        but only one vertex may be selected as start
SWITCH: All valid edges are taken. 
        A process per vertex is spawned and
        previous process is killed.
SWITCH_UNSAGE: Same as SWITCH but will pass edge->b_vars to edge->f
**/
enum CONTEXT
{
        NONE = 0,
        NONE_UNSAFE = 8,
        SINGLE = 1,
        SWITCH = 2,
        SWITCH_UNSAFE = 10,
};

#ifdef __cplusplus
}
#endif

#endif
