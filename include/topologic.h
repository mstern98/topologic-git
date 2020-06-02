#ifndef __TOPOLOGIC__
#define __TOPOLOGIC__

#define _GNU_SOURCE
#include<stdint.h>
#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<err.h>
#include<termios.h>
#include<errno.h>
#include<sys/user.h>
#include<sys/syscall.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<signal.h>
#include<assert.h>
#include<sys/mman.h>
#include<sys/time.h>
#include "./stack.h"
#include "./AVL.h"
#include "./edge.h"
#include "./vertex.h"

/** Graph **/
struct graph{
    enum CONTEXT context;
    struct stack start_set; //CHANGE STRUCTURE?
    struct AVLTree* vertices;
    unsigned int max_state_changes;
    unsigned int snapshot_timestamp;
    unsigned int lvl_verbose;
    sig_atomic_t state; //CURRENT STATE {PRINT, RED, BLACK}
    sig_atomic_t previous_color; //LAST NODE COLOR TO FIRE
    sig_atomic_t print_flag; //0 DID NOT PRINT; 1 FINISHED PRINT
    sig_atomic_t red_node_count; //Number of RED nodes not reaped
    sig_atomic_t black_node_count; //Number of BLACK nodes not reaped
};

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
enum CONTEXT {NONE=0, SINGLE=1, SWITCH=2};

/** 
Enum for memory handling when forking -- if mem amount is exceeded 
ABORT: Kill the program and clean
WAIT: Wait until there is enough memory and try again
CONTINUE: Ignore edge error and move on
**/
enum MEM_OPTION {ABORT=0, WAIT=1, CONTINUE=2};

/**
Enum for state of global manager -- locking when printing and firing;
Ensures proper locking between printing node information and firing information
PRINT: Print, then lock
RED: # Readers = 0 -> unlock print, goes to BLACK at finish
BLACK: Wait until # Readers -> 0, then print
**/
enum STATES{PRINT=0, RED=1, BLACK=2};

#endif