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

struct request *create_request(enum REQUEST request, int argc, void **args, void (*f)(void *)) {
    struct request *req = malloc(sizeof(struct request));
    if (!req) return NULL;
    req->args = malloc(sizeof(void *) * argc);
    if (!req->args) {
        free(req);
        return NULL;
    }
    memcpy(req->args, args, sizeof(void *) * argc);

    req->argc = argc;
    req->f = f;
    req->request = request;
    return req;
}

int submit_request(struct graph *graph, struct request *request) {
    if (!graph || !request) return -1;
    int retval = 0;
    pthread_mutex_lock(&graph->active);
    switch(request->request) {
        case MODIFY:
            retval = push(graph->modify, (void *) request);
            break;
        case DESTROY_EDGE:
            retval = push(graph->remove_edges, (void *) request);
            break;
        case DESTROY_VERTEX:
            retval = push(graph->remove_vertices, (void *) request);
            break;
        default:
            retval = -1;
            break;
    }
    pthread_mutex_unlock(&graph->active);
    return retval;
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




