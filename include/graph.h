/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_GRAPH__
#define __TOPOLOGIC_GRAPH__

#include <pthread.h>
#include <signal.h>

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
**/
enum CONTEXT
{
        NONE = 0,
        SINGLE = 1,
        SWITCH = 2
};

/** 
Enum for memory handling when forking -- if mem amount is exceeded 
ABORT: Kill the program and clean
WAIT: Wait until there is enough memory and try again
CONTINUE: Ignore edge error and move on
**/
enum MEM_OPTION
{
        ABORT = 0,
        WAIT = 1,
        CONTINUE = 2
};

/**
Enum for state of global manager -- locking when printing and firing;
Ensures proper locking between printing node information and firing information
PRINT: Print, then lock
RED: # Readers = 0 -> unlock print, goes to BLACK at finish
BLACK: Wait until # Readers -> 0, then print
**/
enum STATES
{
        PRINT = 0,
        RED = 1,
        BLACK = 2,
        TERMINATE = 3
};

/**
Enum for snapshots. 
NONE: Record nothing
START_STOP: Record first and last state
**/
enum SNAPSHOT
{
        NO_SNAP = -1,
        START_STOP = 0
};

/**
Enum for how verbose the records are
NONE: Record nothing
NODES: Record nodes
EDGES: Record edges
FUNCTIONS: Record the functions of nodes and/or edges
GLOBALS: Record the globals of nodes and/or edges; Also will record shared edges
**/
enum VERBOSITY
{
        NO_VERB = 0,
        VERTICES = 1,
        EDGES = 2,
        FUNCTIONS = 4,
        GLOBALS = 8
};

/** Graph **/
struct graph
{
        enum CONTEXT context;
	enum MEM_OPTION mem_option;
        struct AVLTree *vertices;
        struct stack *start;
        struct stack *modify;
        struct stack *remove_edges;
        struct stack *remove_vertices;
        unsigned int max_state_changes;
        unsigned int max_loop;
        unsigned int snapshot_timestamp;
        enum VERBOSITY lvl_verbose;
        int state_count;
        pthread_t thread;
        pthread_mutex_t lock;
        sig_atomic_t state;          //CURRENT STATE {PRINT, RED, BLACK}
        sig_atomic_t previous_color; //LAST NODE COLOR TO FIRE
        sig_atomic_t print_flag;     //0 DID NOT PRINT; 1 FINISHED PRINT
        pthread_cond_t print_cond;
        sig_atomic_t red_vertex_count; //Number of RED nodes not reaped
        pthread_cond_t red_cond;
        sig_atomic_t black_vertex_count; //Number of BLACK nodes not reaped
        pthread_cond_t black_cond;
};

#endif
