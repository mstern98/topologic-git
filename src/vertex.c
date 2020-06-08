#include "../include/topologic.h"

struct vertex *create_vertex(struct graph *graph, struct vertex_result *(*f)(void *), int id, void *glbl) {
    if (!graph || !f) return NULL;
    pthread_mutex_lock(&graph->lock);

    struct vertex *vertex = malloc(sizeof(struct vertex));
    if (!vertex) {
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->shared = malloc(sizeof(union shared_edge));
    if (!vertex->shared) {
        free(vertex);
        vertex = NULL;
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->edge_tree = init_avl();
    if (!vertex->edge_tree) {
        free(vertex->shared);
        vertex->shared = NULL;
        free(vertex);
        vertex = NULL;
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->joining_vertices = init_avl();
    if (!vertex->joining_vertices) {
        free(vertex->shared);
        vertex->shared = NULL;
        destroy_avl(vertex->edge_tree);
        vertex->edge_tree = NULL;
        free(vertex);
        vertex = NULL;
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (insert(graph->vertices, vertex, id) < 0) {
        free(vertex->shared);
        vertex->shared = NULL;
        destroy_avl(vertex->edge_tree);
        vertex->edge_tree = NULL;
        destroy_avl(vertex->joining_vertices);
        vertex->joining_vertices = NULL;
        free(vertex);
        vertex = NULL;
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    if (pthread_mutex_init(&vertex->lock, NULL) < 0) {
        free(vertex->shared);
        vertex->shared = NULL;
        destroy_avl(vertex->edge_tree);
        vertex->edge_tree = NULL;
        destroy_avl(vertex->joining_vertices);
        vertex->joining_vertices = NULL;
        free(vertex);
        vertex = NULL;
        pthread_mutex_unlock(&graph->lock);
        return NULL;
    }

    vertex->is_active = 0;
    vertex->f = f;
    vertex->id = id;
    vertex->glbl = glbl;

    pthread_mutex_unlock(&graph->lock);
    return vertex;
}

int remove_vertex(struct graph *graph, struct vertex *vertex) {
    if (!graph || !vertex) return -1;
    pthread_mutex_lock(&graph->lock);
    pthread_mutex_lock(&vertex->lock);

    if(!remove_ID(graph->vertices, vertex->id)) {
        pthread_mutex_unlock(&vertex->lock);
        pthread_mutex_unlock(&graph->lock);
        return -1;
    }

    struct stack *stack = init_stack();
    stackify(vertex->edge_tree, stack);
    vertex->edge_tree = NULL;
    struct edge *edge = NULL;
    while ((edge = (struct edge *) pop(stack)) != NULL) {
        pthread_mutex_lock(&(edge->b->lock));
        remove_ID(edge->b->joining_vertices, vertex->id);
        pthread_mutex_unlock(&(edge->b->lock));
        edge->a = NULL;
        edge->b = NULL;
        edge->id = 0;
        edge->f = NULL;
        edge->glbl = NULL;
        free(edge);
        edge = NULL;
    }

    stackify(vertex->joining_vertices, stack);
    vertex->joining_vertices = NULL;
    struct vertex *joining_vertex = NULL;
    while ((joining_vertex = (struct vertex *) pop(stack)) != NULL) {
        remove_edge_id(joining_vertex, vertex->id);
    }

    destroy_stack(stack);
    stack = NULL;

    vertex->id = 0;
    free(vertex->shared);
    vertex->shared = NULL;
    vertex->glbl = NULL;
    
    pthread_mutex_unlock(&vertex->lock);
    pthread_mutex_destroy(&vertex->lock);
    pthread_mutex_unlock(&graph->lock);

    free(vertex);
    vertex = NULL;

    return 0;
}

int modify_vertex(struct vertex *vertex, struct vertex_result *(*f)(void *), void *glbl) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    if (f != NULL) {
        vertex->f = f;
    }
    if (glbl != NULL) { 
        vertex->glbl = glbl;
    }

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

int modify_shared_edge_vars(struct vertex *vertex, void *edge_vars) {
    if (!vertex) return -1;
    pthread_mutex_lock(&vertex->lock);

    vertex->shared->vertex_data = edge_vars;

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

