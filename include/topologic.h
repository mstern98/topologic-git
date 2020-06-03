#ifndef __TOPOLOGIC__
#define __TOPOLOGIC__

#define _GNU_SOURCE
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "./stack.h"
#include "./AVL.h"
#include "./edge.h"
#include "./vertex.h"


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
    BLACK = 2
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
    NODES = 1,
    EDGES = 2,
    FUNCTIONS = 4,
    GLOBALS = 8
};

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

/** Graph **/
struct graph
{
    enum CONTEXT context;
    struct stack *start_set; //CHANGE STRUCTURE?
    struct AVLTree *vertices;
    struct stack *modify;
    struct stack *remove_edges;
    struct stack *remove_vertices;
    unsigned int max_state_changes;
    unsigned int snapshot_timestamp;
    enum VERBOSITY lvl_verbose;
    pthread_mutex_t lock;
    sig_atomic_t state;            //CURRENT STATE {PRINT, RED, BLACK}
    sig_atomic_t previous_color;   //LAST NODE COLOR TO FIRE
    sig_atomic_t print_flag;       //0 DID NOT PRINT; 1 FINISHED PRINT
    pthread_cond_t print_cond;
    sig_atomic_t red_vertex_count;   //Number of RED nodes not reaped
    pthread_cond_t red_cond;
    sig_atomic_t black_vertex_count; //Number of BLACK nodes not reaped
    pthread_cond_t black_cond;
};

/**
@PARAM max_state_changes: # state changes before entering 
                          sink vertex due to infinite loop of states; -1 to ignore 
@PARAM snapshot_timestamp: printing out data at given timestamp for user; -1 for none; 0 for first and last state
@PARAM lvl_verbose: how verbose timestamp print is
@PARAM context: linear or context-switch based
@RETURN an empty graph
Creates a graph structures
**/
struct graph *graph_init(unsigned int max_state_changes,
                         unsigned int snapshot_timestamp,
                         enum VERBOSITY lvl_verbose,
                         enum CONTEXT context);
#define MAX_STATE_CHANGES 100
#define GRAPH_INIT graph_init(MAX_STATE_CHANGES, START_STOP, NODES|EDGES|FUNCTIONS|GLOBALS, SWITCH)

/**
@PARAM graph: the graph
@PARAM f: a function
@PARAM id: vertex id to be used: MUST BE UNIQUE
@PARAM argc: # of arguments f takes
@PARAM glblc: # of global variables
@PARAM glbl: global variables
@RETURN vertex: a vertex to be used in a graph
On creation a process will be spawned for the vertex
The vertex will compute function f when called
NOTE: NULL glbl will mean no global variables. 
      f cannot be NULL.
**/

struct vertex *create_vertex(struct graph *graph,
                             void (*f)(void *),
                             int id,
                             int argc,
                             int glblc,
                             void **glbl);
#define create_vertex(graph, f, id, argc) create_vertex(graph, f, id, argc, 0, NULL)

/**
@PARAM a: A vertex
@PARAM b: Another vertex (can be 'a')
@PARAM f: a function
@PARAM argc: # of arugments f takes
@PARAM glblc: # of global variables
@PARAM glbl: global variables
@RETURN the edge connecting a to b
Will create an edge from vertex a to b
with some criteria determined by the function f.
NOTE: NULL glbl will mean no global variables. f cannot be NULL.
**/
struct edge *create_edge(struct vertex *a,
                         struct vertex *b,
                         void (*f)(void *),
                         int argc,
                         int glblc,
                         void **glbl);
#define create_edge(id, a, b, f, argc) create_edge(id, a, b, f, argc, 0, NULL)

/**
@RETURN two edges
See create_edge
Will create an bidirectional edge between vertex a and b
with some criteria determined by the function f.
**/
struct edge **create_bi_edge(struct vertex *a,
                             struct vertex *b,
                             void (*f)(void *),
                             int argc,
                             int glblc,
                             void **glbl);
#define create_bi_edge(a, b, f, argc) create_bi_edge(a, b, f, argc, 0, NULL)

/**
@PARAM a: a vertex
@PARAM b: another vertex
@RETURN 0 for success;
        -1 for fail
Removes the edge connecting a to b
**/
int remove_edge(struct vertex *a,
                struct vertex *b);

/**
@PARAM a: a vertex
@PARAM b: another vertex
@RETURN  0 for success; 
        -1 for fail;
        -2 if only edge from a to b is removed; 
        -3 if only edge from b to a is removed
Removes the edge connecting a to b
**/
int remove_bi_edge(struct vertex *a,
                   struct vertex *b);

/**
@PARAM graph: the graph
@PARAM vertex: a vertex
@RETURN 0 for sucess;
        -1 for fail
Removes the vertex and all connected edges
**/
int remove_vertex(struct graph *graph, 
                  struct vertex *vertex);

/**
@PARAM vertex: a vertex
@PARAM f: a function
@PARAM argc: number of arguments f takes
@PARAM glblc: # of global variables
@PARAM glbl: global variables
NOTE: NULL f, or glbl will mean no change.
@RETURN 0 for success;
        -1 for fail
Modifies the vertices function
**/
int modify_vertex(struct vertex *vertex,
                  void (*f)(void *),
                  int argc,
                  int glblc,
                  void **glbl);
#define modify_vertex(vertex, f, argc) modify_vertex(vertex, f, argc, 0, NULL)
#define modify_vertex_globals(vertex, glblc, glbl) modify_vertex(vertex, NULL, 0, glblc, glbl)

/**
@PARAM vertex: a vertex
@PARAM edgec: # of variables
@PARAM edge_vars: shared variables
@RETURN 0 for success;
        -1 for fail
Modifies the vertices shared variables with it's edges
**/
int modify_shared_edge_vars(struct vertex *vertex, 
                            int edgec, 
                            void **edge_vars);

/**
@PARAM a: a vertex
@PARAM b: another vertex
@PARAM f: a function
@PARAM argc: number of arguments f takes
@PARAM glblc: # of global variables
@PARAM glbl: global variables
@RETURN 0 for success;
        -1 for fail
Modifies the edge connecting a to b's function
NOTE: NULL f, or glbl will mean no change.
**/
int modify_edge(struct vertex *a,
                struct vertex *b,
                void (*f)(void *),
                int argc,
                int glblc,
                void **glbl);

#define modify_edge(a, b, f, argc) modify_edge(a, b, f, argc, 0, NULL)
#define modify_edge_global(a, b, glblc, glbl) modify_edge(a, b, NULL, 0, glblc, glbl)

/**
@PARAM a: a vertex
@PARAM b: another vertex
@PARAM f: a function
@PARAM argc: number of arguments f takes
@PARAM glblc: # of global variables
@PARAM glbl: global variables
@RETURN 0 for success;
        -1 for fail;
        -2 if only edge from a to b is modified; 
        -3 if only edge from b to a is modified 
Modifies the edge between a and b's
function and variables
NOTE: NULL f, or glbl will mean no change.
**/
int modify_bi_edge(struct vertex *a,
                   struct vertex *b,
                   void (*f)(void *),
                   int argc,
                   int glblc,
                   void **glbl);
#define modify_bi_edge(a, b, f, argc) modify_bi_edge(a, b, f, argc, 0, NULL)
#define modify_bi_edge_global(a, b, glblc, glbl) modify_bi_edge(a, b, NULL, 0, glblc, glbl)


/**
@PARAM graph: the graph
@PARAM vertex: A vertex to be ran
@PARAM argc: # of arguments
@PARAM args: arguments
@PARAM color: the state in which unlocks fire process
              if STATE is set to PRINT then
              fire will fail
@RETURNS the result of the vertex
fire will wake up the vertex and pass 
argc and args to the vertex to 
compute its function and then 
call switch and clean itself up
**/
void fire(struct graph *graph,
          struct vertex *vertex,
          int argc,
          void **args,
          enum STATES color);

/**
@PARAM graph: the graph
@PARAM vertex: The vertex in which just fire
@PARAM args; The result of the vertex
Upon call the switch function will 
compute the edge functions 
connected to the vertex
@RETURNS 0 On success; the vertex connected to the
    successful edge will be fired; -1 on failure
**/
int switch_vertex(struct graph *graph,
                  struct vertex *vertex,
                  void **args);

/**
@PARAM graph: the graph,
@PARAM vertices: the set of vertices to start at
@PARAM num_vertices: number of vertices
@RETURN -1 for fail if any vertex fails;
        0 for success 
Creates multiple contexts that are ran in parallel
**/
int start_set(struct graph *graph,
              struct vertex **vertices,
              int num_vertices);

/**
@PARAM graph: the graph
@PARAM request: the request to be processed
@RETRUN -1 for fail;
        0 for succes;
Submits a request to be processed after all active nodes 
complete
**/
int submit_request(struct graph *graph, 
                   struct request *request);

/**
@PARAM request: the desired request
@PARAM args: the arguments needed for f
@PARAM f: the function of the request
@PARAM argc: the number of args f takes
@RETURN the request or NULL if it fails
Creates a request structure to be called later
**/
struct request *create_request(enum REQUESTS request,  
                               void **args, 
                               void (*f)(void *),
                               int argc);

#define create_request_destroy_edge(args) create_request(DESTROY_EDGE, args, remove_edge, 2);
#define create_request_destroy_bi_edge(args) create_request(DESTROY_EDGE, args, remove_bi_edge, 2);
#define create_request_destroy_vertex(args) create_request(DESTROY_VERTEX, args, remove_vertex, 2);

#define create_request_modify_edge(args) create_request(MODIFY, args, modify_edge, 6);
#define create_request_modify_bi_edge(args) create_request(MODIFY, args, modify_bi_edge, 6);
#define create_request_modify_vertex(args) create_request(MODIFY, args, modify_vertex, 5);
#define create_request_modify_shared_edge_vars(args) create_request(MODIFY, args, modify_shared_edge_vars, 3);
/**
@PARAM graph: the graph
Attempts to run the graph else aborts.
**/
void run(struct graph *graph);

/**
@PARAM graph: the graph
Prints the graph with desired output
**/
void print(struct graph *graph);

/**
@PARAM graph: the graph
@RETURN -1 for fail;
        0 for success
Destroys and frees the graph
**/
int destroy_graph(struct graph *graph);

/**
@PARAM request: a request
@RETURN -1 for fail;
        0 for success
Destroys and frees a request
**/
int destroy_request(struct request *request);

#endif
