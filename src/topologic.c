#include "./include/topologic.h"

struct graph *graph_init(unsigned int max_state_changes, unsigned int snapshot_timestamp, enum VERBOSITY lvl_verbose, enum CONTEXT context) {
    struct graph *graph = malloc(sizeof(struct graph));
    if (!graph) return NULL;
    graph->max_state_changes = max_state_changes;
    graph->snapshot_timestamp = snapshot_timestamp;
    graph->lvl_verbose = lvl_verbose;
    graph->context = context;

    if(pthread_mutex_init(&graph->lock) < 0) {
        free(graph);
        return NULL;
    }

    graph->vertices = init_avl();
    if(!graph->vertices) {
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }

    graph->modify = init_stack();
    if (!graph->modify) {
        destroy_avl(graph->vertices);
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }
    graph->remove_edges = init_stack();
    if (!graph->remove_edges) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }
    graph->remove_vertices = init_stack();
    if (!graph->remove_vertices) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        destroy_stack(graph->remove_edges);
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }
    graph->start_set = init_stack();
    if (!graph->start_set) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        destroy_stack(graph->remove_edges);
        destroy_stack(graph->remove_vertices);
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }

    graph->previous_color = RED;
    graph->STATE = RED;
    graph->red_node_count = 0;
    graph->black_node_count = 0;
    graph->print_flag = 0;
    
    return graph;
}

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

struct request *create_request(enum REQUESTS request, int argc, void **args, void (*f)(void *)) {
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

void fire(struct graph *graph, struct vertex *vertex, int argc, void **args, enum STATES color) {
    return 0;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, void **args) {
    return 0;
}

void destroy_graph_stack(struct stack *stack) {
    if (!stack) return;
    struct request *request = NULL;
    while ((request = (struct request *) pop(stack)) != NULL) {
        destroy_request(request);
    }
    destroy_stack(stack);
}

void destroy_graph_avl(struct graph *graph, struct AVLTree *tree) {
    if (!tree) return;
    struct stack *stack = preorder(tree);
    struct vertex *vertex = NULL;
    while ((vertex = (struct vertex *) pop(stack)) != NULL) {
        remove_vertex(graph, vertex);
    }
    destroy_avl(tree);
}

int destroy_graph(struct graph *graph) {
    if (!graph) return -1;
    destroy_graph_avl(graph, graph->vertices);
    destroy_graph_stack(graph->modify);
    destroy_graph_stack(graph->remove_edges);
    destroy_graph_stack(graph->remove_vertices);

    pthread_mutex_destroy(&graph->lock);
    free(graph);
    return 0;
}

int destroy_request(struct request *request) {
    if (!request) return -1;
    if (request->argc == 1) {
        free(request->args);
        request->args = NULL;
    } else if (request->argc > 1) {
        int i = 0;
        for (i; i < request->argc; i++) {
            free(request->args[i]);
            request->args[i] = NULL;
        }
        free(request->args);
    }
    request->argc = 0;
    request->requesut = 0;
    return 0;
}




