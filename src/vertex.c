#include "../include/topologic.h"

struct vertex *create_vertex(struct graph *graph, void (*f)(void *), int id, int argc, int glblc, void *glbl) {
    return NULL;
}

int remove_vertex(struct graph *graph, struct vertex *vertex) {
    return 0;
}

int modify_vertex(struct graph *graph, struct vertex *vertex, void (*f)(void *), int argc, int glblc, void *glbl) {
    return 0;
}

int modify_shared_edge_vars(struct graph *graph, struct vertex *vertex, int edgec, void *edge_vars) {
    return 0;
}

