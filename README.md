# Topologic
A library to simulate DFAs and Probabilistic DFAs using context switching. This library provides a way to build and define the behavior of a graph. The client is able to define the function each vertex and edge of the graph make and how the graph transitions from one (the first valid edge) or to many states, as well as which states the graph starts in.

The library provides a way for the client to define what information is saved localy to each vertex and edge, as well as which information the vertex and edges originating at that edge share. In addition the client can pass information to each initial state that will be passed and modifyable by vertices at each state transition. 

This library can be used to create AI, ML, and simulations (such as of stock options, electron-electron interaction, etc.).

## Installation
### Arch Linux
>yay -S topologic-git

-To use in program use

>#include <topologic/topologic.h>

-Link with

>gcc ... -ltopologic

### Manualy
Build C Library
>make

-creates libtopologic.a

Build C++ Library
>make cpp

-creates libtopologic.a

Clean
>make clean

Debug
>make CFLAGS=-DDEBUG

## TODO
-Thread sanity

-C# Wrap

-Rust Wrap

-Python (2/3) Wrap

# Errata
## Edge/vertex modification
Modifying/deleting vertices and edges inside f() not using submit request
can lead to undefined behavior or dead locks. This is because in CONTEXT
set to SWITCH or NONE with many starting vertices, can lead to a structure
being NULL’d with its lock destroyed while another thread is holding that lock
or two threads trying to modify each other and thus resulting in a dead lock. It
is up to the client’s discretion to modify directly when in SWITCH or NONE
with many starting vertices. Although in SINGLE, this should be fine. To
mitigate this problem, the client should use submit request which will handle
the requests sequentially with destroying structures last.

## Graph modification
Trying to delete the graph while running will result in undefined behav-
ior. Destroying the graph does not lock any thread and thus will cause race
conditions. The graph should be deleted only once all threads reach a sink.

## Parameter Passing
Parameters passed to edges or vertices functions will be free’d immediately
after use and therefore will cause an error should they try and be accessed. To
mitigate the values may be stored in the edge’s or vertex’s global or shared
variables. Any non standard data type, such as struct, is dependent on the user
to free its content as the library will only free the pointer to the struct and the
pointer to the array of variables.

## Illegal Arguments
Passing wrong values or wrong number of variables to any function will result
in failure. A client should be aware of which vertices connect and what edges
it has and the proper handling required between such connections. Should the
client choose to dynamically add/remove vertices or edges or even modify while
running should be aware of the changes that may occur in the graph and the
resulting change in dependence on proper variable handling.

## Graph Modification (cont.)
It is possible to modify the graph while it is running. To do so the client
should submit request to add a change or pause the graph. Making any changes
externally may result in undefined behavior if done improperly. Modifying the
graph directly while running could result in failure.
	Should the number of init vertex args not much the number of vertices in
the start set the program will result in an error.

## Data Structure Errata
Stacks and AVLTree can take non malloc’d data and function normally
within scope of those non malloc’d data. However, in another scope the memory
will be unaddressable and thus should be malloc’d memory instead. The stack
and AVL Tree wll not free the void * data since the void * data structure is
unknown to them and thus the client should free the memory.

## SWITCH_UNSAFE
Should SWITCH_USNAFE be used as the context, then the graph will run like
how it runs in SWITCH, but the variables shared between the edge and the second vertex
b will be passed to the edges function. Trying to read the shared variables may lead to a 
race condition in which vertex b or one/some of it's edges are modifying those varibales.
The client is expected to handle such race conditions, and failing to do so can lead to a possibly
unsafe execution. Thus this mode is UNSAFE. It is safe to use the shared variables with vertex b
in context modes NONE and SINGLE since in NONE vertex b cannot also be active, and in SINGLE there 
is no threading. 

