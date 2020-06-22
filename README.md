# Topologic
A library to simulate DFAs and Probabilistic DFAs using context switching. This library provides a way to build and define the behavior of a graph. The client is able to define the function each vertex and edge of the graph make and how the graph transitions from one (the first valid edge) or to many states, as well as which states the graph starts in.

The library provides a way for the client to define what information is saved localy to each vertex and edge, as well as which information the vertex and edges originating at that edge share. In addition the client can pass information to each initial state that will be passed and modifyable by vertices at each state transition. 

This library can be used to create AI, ML, and simulations (such as of stock options, electron-electron interaction, etc.).

.zip [button url="https://github.com/mstern98/Topologic/releases/latest/archive/v1.0.0.zip"]
.tar.gz [button url="https://github.com/mstern98/Topologic/releases/latest/archive/v1.0.0.tar.gz"]

## Installation
Build C Library
>make

-creates libtopologic.a

Build C++ Library
>make cpp

-creates libtopologic.a

Build Python 3 Library
>make python

-creates topylogic/topylogic.so

Build Python 2 Library
>make python2

-creates topylogic/topylogic.so

Clean
>make clean

## TODO
-C# Wrap

-Rust Wrap

-Python (2/3) Wrap
