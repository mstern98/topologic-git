#include "../include/topologic.h"

struct vertex *create_vertex(struct graph *graph, void (*f)(void *), int id, int argc, int glblc, void **glbl) {
    if (!graph || !f) return NULL;
    pthread_mutex_lock(&graph->lock);

    struct vertex *vertex = malloc(sizeof(struct vertex));
    if (!vertex) {
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (insert(graph->vertices, (void **)(&vertex), id) < 0) {
        free(vertex);
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (pthread_mutex_init(&vertex->lock, NULL) < 0) {
        free(vertex);
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->f = f;
    vertex->argc = argc;
    vertex->id = id;
    vertex->edge_sharedc = 1;
    vertex->edge_shared = malloc(sizeof(void *));
    vertex->edge_shared = 0;
    vertex->edge_tree = init_avl();
    vertex->glblc = glblc;
    if (glblc > 0) {
        vertex->glbl = malloc(sizeof(void *) *glblc);
        memcpy(vertex->glbl, *glbl, sizeof(void *) * glblc);
    }

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
    vertex->argc = 0;

    int i = 0;
    if (vertex->edge_sharedc > 1) {
        for (i = 0; i < vertex->edge_sharedc; i++) {
            free(vertex->edge_shared[i]);
            vertex->edge_shared[i] = NULL;
        } 
        free(vertex->edge_shared);
        vertex->edge_shared = NULL;
    } else if (vertex->edge_sharedc == 1) {
        free(vertex->edge_shared);
        vertex->edge_shared = NULL;
    }
    if (vertex->glblc > 1) {
        for (i = 0; i < vertex->glblc; i++) {
            free(vertex->glbl[i]);
            vertex->glbl[i] = NULL;
        }
        free(vertex->glbl);
        vertex->glbl = NULL;
    } else if (vertex->glblc == 1) {
        free(vertex->glbl);
        vertex->glbl = NULL;
    }
    

    struct stack *stack = stackify(vertex->edge_tree);
    struct edge *edge = NULL;
    while ((edge = (struct edge *) pop(stack)) != NULL) {
        edge->a = NULL;
        edge->b = NULL;
        edge->id = 0;
        edge->argc = 0;
        edge->f = NULL;
        if (edge->glblc > 1) {
            for (i = 0; i < edge->glblc; i++) {
                free(edge->glbl[i]);
                edge->glbl[i] = NULL;
            }
                free(edge->glbl);
                edge->glbl = NULL;
        } else if (edge->glblc == 1) {
            free(edge->glbl);
            edge->glbl = NULL;
        }
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

int modify_vertex(struct vertex *vertex, void (*f)(void *), int argc, int glblc, void **glbl) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    if (f != NULL) {
        vertex->f = f;
        vertex->argc = argc;
    }
    if (glbl != NULL) {
        if (vertex->glblc > 1) {
            for (i = 0; i < vertex->glblc; i++) {
                free(vertex->glbl[i]);
                vertex->glbl[i] = NULL;
            }
            free(vertex->glbl);
            vertex->glbl = NULL;
        } else if (vertex->glblc == 1) {
            free(vertex->glbl);
            vertex->glbl = NULL;
        }
        vertex->glblc = glblc;
        if (glblc > 0) {
            vertex->glbl = malloc(sizeof(void *) *glblc);
            memcpy(vertex->glbl, *glbl, sizeof(void *) * glblc);
        }
    }

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

int modify_shared_edge_vars(struct vertex *vertex, int edgec, void **edge_vars) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    void *data = realloc(vertex->edge_shared, sizeof(void *) * edgec);
    if (!data) {
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    vertex->edge_sharedc = edgec;
    vertex->edge_shared = data;
    memcpy(vertex->edge_shared, *edge_vars, sizeof(void *) * edgec);

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

