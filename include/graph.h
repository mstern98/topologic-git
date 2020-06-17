/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_GRAPH__
#define __TOPOLOGIC_GRAPH__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "./windows_wrap.h"
#else
#include <pthread.h>
#endif

#include <signal.h>
#include "./context.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define MAX_ATTEMPTS 4         //Maximum attempts for system to check for pthread if WAIT enum is selected
#define THREAD_ATTEMPT_SLEEP 3 //Sleep amount before trying to create thread again for WAIT

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
        int max_state_changes;
        int max_loop;
        int snapshot_timestamp;
        unsigned int lvl_verbose;
        int state_count;
        pthread_mutex_t lock;
        pthread_mutex_t color_lock;
        sig_atomic_t state;            //CURRENT STATE {PRINT, RED, BLACK}
        sig_atomic_t previous_color;   //LAST NODE COLOR TO FIRE
        sig_atomic_t print_flag;       //0 DID NOT PRINT; 1 FINISHED PRINT
        sig_atomic_t red_vertex_count; //Number of RED nodes not reaped
        sig_atomic_t black_vertex_count; //Number of BLACK nodes not reaped
        sig_atomic_t pause;
        sig_atomic_t red_locked;
        sig_atomic_t black_locked;
        sig_atomic_t num_vertices;
        pthread_cond_t pause_cond;
        pthread_cond_t red_fire;
        pthread_cond_t black_fire;
};

#ifdef __cplusplus
}
#endif

#endif
