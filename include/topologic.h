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
#include "./request.h"
#include "./graph.h"

struct fireable{
        struct graph *graph;
        struct vertex *vertex;
        void *args;
        enum STATES color;
};

/**
 Wrapper function for fire, fire_1
@PARAM vargp: arguments 
**/
void* fire_pthread(void* vargp);

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
#define GRAPH_INIT() graph_init(MAX_STATE_CHANGES, START_STOP, VERTICES | EDGES | FUNCTIONS | GLOBALS, SWITCH)

/**
@PARAM graph: the graph
@PARAM f: a function
@PARAM id: vertex id to be used: MUST BE UNIQUE
@PARAM glbl: global variables
@RETURN vertex: a vertex to be used in a graph
On creation a process will be spawned for the vertex
The vertex will compute function f when called
NOTE: NULL glbl will mean no global variables. 
      f cannot be NULL.
**/
struct vertex *create_vertex(struct graph *graph,
                             struct vertex_result *(*f)(void *),
                             int id,
                             void *glbl);
#define CREATE_VERTEX(graph, f, id) create_vertex(graph, f, id, NULL)

/**
@PARAM a: A vertex
@PARAM b: Another vertex (can be 'a')
@PARAM f: a function
@PARAM glbl: global variables
@RETURN the edge connecting a to b
Will create an edge from vertex a to b
with some criteria determined by the function f.
NOTE: NULL glbl will mean no global variables. f cannot be NULL.
**/
struct edge *create_edge(struct vertex *a,
                         struct vertex *b,
                         int (*f)(void *),
                         void *glbl);
#define CREATE_EDGE(a, b, f) create_edge(a, b, f, NULL)

/**
@RETURNS 0 for success;
        -1 for fail
See create_edge
Will create an bidirectional edge between vertex a and b
with some criteria determined by the function f.
Will store the edges in edge_a and edge_b.
If edge_a_to_b or edge_b_to_a is NULL it will not.
**/
int create_bi_edge(struct vertex *a,
                             struct vertex *b,
                             int (*f)(void *),
                             void *glbl,
                             struct edge **edge_a_to_b,
                             struct edge **edge_b_to_a);
#define CREATE_NULL_BI_EDGE(a, b, f) create_bi_edge(a, b, f, NULL, NULL, NULL)
#define CREATE_BI_EDGE(a, b, f, a_to_b, b_to_a) create_bi_edge(a, b, f, NULL, a_to_b, b_to_a)

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
@PARAM id: id of edge to remove
@RETURN 0 for success;
        -1 for fail
Removes the edge in a with that id
**/
int remove_edge_id(struct vertex *a,
                   int id);
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
@PARAM glbl: global variables
NOTE: NULL f, or glbl will mean no change.
@RETURN 0 for success;
        -1 for fail
Modifies the vertices function
**/
int modify_vertex(struct vertex *vertex,
                  struct vertex_result *(*f)(void *),
                  void *glbl);
#define MODIFY_VERTEX(vertex, f) modify_vertex(vertex, f, NULL)
#define MODIFY_VERTEX_GLOBALS(vertex, glbl) modify_vertex(vertex, NULL, glbl)

/**
@PARAM vertex: a vertex
@PARAM edge_vars: shared variables
@RETURN 0 for success;
        -1 for fail
Modifies the vertices shared variables with it's edges
**/
int modify_shared_edge_vars(struct vertex *vertex,
                            void *edge_vars);

/**
@PARAM a: a vertex
@PARAM b: another vertex
@PARAM f: a function
@PARAM glbl: global variables
@RETURN 0 for success;
        -1 for fail
Modifies the edge connecting a to b's function
NOTE: NULL f, or glbl will mean no change.
**/
int modify_edge(struct vertex *a,
                struct vertex *b,
                int (*f)(void *),
                void *glbl);

#define MODIFY_EDGE(a, b, f) modify_edge(a, b, f, NULL)
#define MODIFY_EDGE_GLOBALS(a, b, glbl) modify_edge(a, b, NULL, glbl)

/**
@PARAM a: a vertex
@PARAM b: another vertex
@PARAM f: a function
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
                   int (*f)(void *),
                   void *glbl);
#define MODIFY_BI_EDGE(a, b, f) modify_bi_edge(a, b, f, NULL)
#define MODIFY_BI_EDGE_GLOBALS(a, b, glbl) modify_bi_edge(a, b, NULL, glbl)

/**
@PARAM graph: the graph
@PARAM vertex: A vertex to be ran
@PARAM args: arguments
@PARAM color: the state in which unlocks fire process
              if STATE is set to PRINT then
              fire will fail
@RETURNS the result of the vertex
fire will wake up the vertex and pass 
args to the vertex to 
compute its function and then 
call switch and clean itself up
**/
int fire(struct graph *graph,
         struct vertex *vertex,
         void *args,
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
                  void *args,
                  enum STATES color);

/**
@PARAM graph: the graph,
@PARAM id: the ids of the vertices
@PARAM num_vertices: number of vertices
@RETURN -1 for fail if any vertex fails;
        0 for success 
Creates multiple contexts that are ran in parallel
**/
int start_set(struct graph *graph,
              int *id,
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
@RETURN the request or NULL if it fails
Creates a request structure to be called later
**/
struct request *create_request(enum REQUESTS request,
                               void *args,
                               void (*f)(void *));

#define CREATE_REQUEST(request, args) create_request(request, args, NULL)


/**
@PARAM graph: the graph
Process requests that are queued in the graph
**/
void process_requests(struct graph *graph);

/**
@PARAM graph: the graph
@PARAM vertex_args: array of vertex arguments for f
Attempts to run the graph else aborts.
**/
void run(struct graph *graph,
         void **vertex_args);

/**
@PARAM graph: the graph
Prints the graph with desired output
**/
void print(struct graph *graph);

/**
@PARAM file: graph input file
@RETURN the graph specified in the file
**/
struct graph *parse_json(FILE *file);

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
