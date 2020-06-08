#include "../include/topologic.h"

struct vertex *create_vertex(struct graph *graph, struct vertex_result *(*f)(void *), int id, int glblc, void *glbl) {
    if (!graph || !f) return NULL;
    pthread_mutex_lock(&graph->lock);

    struct vertex *vertex = malloc(sizeof(struct vertex));
    if (!vertex) {
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (insert(graph->vertices, vertex, id) < 0) {
        free(vertex);
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (pthread_mutex_init(&vertex->lock, NULL) < 0) {
        free(vertex);
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->is_active = 0;
    vertex->f = f;
    vertex->id = id;
    vertex->edge_shared = malloc(sizeof(void));
    vertex->edge_sharedc = 0;
    vertex->edge_tree = init_avl();
    vertex->glblc = glblc;
    vertex->glbl = glbl;

    pthread_mutex_unlock(&graph->lock);
    return vertex;
}

int remove_vertex(struct graph *graph, struct vertex *vertex) {
    if (!graph || !vertex) return -1;
    pthread_mutex_lock(&graph->lock);
    pthread_mutex_lock(&vertex->lock);

    if(!remove_ID(graph->vertices, vertex->id)) {
        pthread_mutex_unlock(&graph->lock);
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    vertex->id = 0;

    free(vertex->edge_shared);
    vertex->edge_shared = NULL;

    free(vertex->glbl);
    vertex->glbl = NULL;

    struct stack *stack = init_stack();
    stackify(vertex->edge_tree, stack);
    free(vertex->edge_tree);
    vertex->edge_tree = NULL;
    struct edge *edge = NULL;
    while ((edge = (struct edge *) pop(stack)) != NULL) {
        edge->a = NULL;
        edge->b = NULL;
        edge->id = 0;
        edge->f = NULL;
        free(edge->glbl);
        edge->glbl = NULL;
        free(edge);
        edge = NULL;
    }

    destroy_stack(stack);
    stack = NULL;
    
    pthread_mutex_unlock(&vertex->lock);
    pthread_mutex_destroy(&vertex->lock);
    pthread_mutex_unlock(&graph->lock);

    free(vertex);
    vertex = NULL;

    return 0;
}

int modify_vertex(struct vertex *vertex, struct vertex_result *(*f)(void *), int glblc, void *glbl) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    if (f != NULL) {
        vertex->f = f;
    }
    if (glbl != NULL) { 
        free(vertex->glbl);
        vertex->glbl = NULL;
        vertex->glblc = glblc;
        if (glblc > 0) {
            vertex->glbl = glbl;
        }
    }

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

int modify_shared_edge_vars(struct vertex *vertex, int edgec, void *edge_vars) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    void *data = realloc(vertex->edge_shared, sizeof(void) * edgec);
    if (!data) {
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    vertex->edge_sharedc = edgec;
    vertex->edge_shared = data;
    vertex->edge_shared = edge_vars;

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

