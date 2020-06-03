#include "./include/topologic.h"

int start_set(struct graph *graph, struct vertex **vertices, int num_vertices) {
    if(!graph) return -1;
    if(!vertices) return -1;
    if(num_vertices<0) return -1;

    int i = 0;
    for(;i<num_vertices; i++){
        struct vertex* v = vertices[i];
        if(!v) return -1;
        fire(graph, v, v->argc, v->edge_shared, RED);
    }
    return 0;
}

void run(struct graph *graph) {

}

void print(struct graph *graph) {

}

void fire(struct graph *graph, struct vertex *vertex, int argc, void *args, enum STATES color) {
    return 0;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, void *args) {
    return 0;
}

int clean_graph(struct graph *graph) {
    return 0;
}




