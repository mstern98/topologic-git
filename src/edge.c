#include "../include/topologic.h"

struct edge *create_edge(struct graph *graph, int id, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    return NULL;       
}

struct edge **create_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    return NULL;
}

int remove_edge(struct graph *graph, struct vertex *a, struct vertex *b) {
    return 0;
}

int remove_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b) {
    return 0;
}

int modify_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc,int glblc, void *glbl) {
    return 0;
}

int modify_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc,int glblc, void *glbl) {
    return 0;
}