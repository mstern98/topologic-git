#include "../include/topologic.h"

struct edge *create_edge(struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl) {
    if(!a || !b){ return NULL; }
    if(!f){ return NULL; }

    pthread_mutex_lock(&a->lock); 
    void *exists = find(a->edge_tree, b->id);
    if (exists) {
        pthread_mutex_unlock(&a->lock); 
        return NULL; 
    }

    struct edge* edge = malloc(sizeof(struct edge));
    if(!edge){ 
        pthread_mutex_unlock(&a->lock); 
        return NULL;
    }
    edge->a = a;
    edge->b = b;

    edge->glbl = glbl;

    edge->a_vars = a->shared->edge_data;

    edge->f = f;
    //MAKE UNIQUE
    edge->id = b->id;

    if(insert(a->edge_tree, edge, edge->id) < 0){
        edge->a = NULL;
        edge->b = NULL;
        edge->a_vars = NULL;
        edge->f = NULL;
        edge->glbl = NULL;
        edge->id = 0;
        free(edge);
        edge = NULL;
        pthread_mutex_unlock(&a->lock); 
        return NULL;
    }

    pthread_mutex_lock(&b->lock);
    if (insert(b->joining_vertices, a, a->id) < 0) {
        remove_ID(a->edge_tree, edge->id);
        edge->a = NULL;
        edge->b = NULL;
        edge->a_vars = NULL;
        edge->f = NULL;
        edge->glbl = NULL;
        edge->id = 0;
        free(edge);
        edge = NULL;
        pthread_mutex_unlock(&b->lock);
        pthread_mutex_unlock(&a->lock); 
    }
    pthread_mutex_unlock(&b->lock);

    pthread_mutex_unlock(&a->lock); 
    return edge;
}

int create_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl, struct edge **edge_a_to_b, struct edge **edge_b_to_a) {
    if(!a || !b || !f || a == b) return -1;
    struct edge *a_to_b, *b_to_a;
    
    a_to_b = create_edge(a, b, f, glbl);
    if (!a_to_b) {
        return -1;
    }

    b_to_a = create_edge(b, a, f, glbl);
    if (!b_to_a) {
        remove_edge(a, b);
        free(a_to_b);
        a_to_b = NULL;
        return -1;
    }

    if (edge_a_to_b) *edge_a_to_b = a_to_b;
    if (edge_b_to_a) *edge_b_to_a = b_to_a;
    return 0;
}

int remove_edge(struct vertex *a, struct vertex *b) {
    if (!a || !b) return -1;

    remove_ID(b->joining_vertices, a->id);

    pthread_mutex_lock(&a->lock); 
    void *data = remove_ID(a->edge_tree, b->id);
    if (!data) {
        pthread_mutex_unlock(&a->lock); 
        return -1;
    }
    
    struct edge *edge = (struct edge *) data;
    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    edge->glbl = NULL;
    
    free(edge);
    edge = NULL;
    pthread_mutex_unlock(&a->lock); 
    return 0;
}

int remove_edge_id(struct vertex *a, int id) {
    if (!a) return -1;
    pthread_mutex_lock(&a->lock); 
    void *data = remove_ID(a->edge_tree, id);
    if (!data) return -1;
    struct edge *edge = (struct edge *) data;

    remove_ID(edge->b->joining_vertices, a->id);

    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;
    edge->glbl = NULL;
    
    free(edge);
    edge = NULL;
    pthread_mutex_unlock(&a->lock); 
    return 0;
}

int remove_bi_edge(struct vertex *a, struct vertex *b) {
    if (!a || !b || a == b) return -1;
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = remove_edge(a, b)) < 0) ret = -2;
    if ((b_ret = remove_edge(b, a)) < 0 && a < 0) ret = -1;
    if (b_ret < 0 && a_ret == 0) ret = -3;

    return ret;
}

int modify_edge(struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl) {
    if (!a || !b) return -1;
    pthread_mutex_lock(&a->lock); 
    struct edge *edge = (struct edge *) find(a->edge_tree, b->id);
    if (!edge) return -1;
    if (f) {
        edge->f = f;
    }
    if (glbl) {
        edge->glbl = glbl;
    }
    pthread_mutex_unlock(&a->lock); 
    return 0;
}

int modify_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl) {
    if(!a || !b || a == b) return -1;
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = modify_edge(a, b, f, glbl)) < 0) ret = -2;
    if ((b_ret = modify_edge(b, a, f, glbl)) < 0 && a < 0) ret = -1;
    if (b_ret < 0 && a_ret == 0) ret = -3;

    return ret;
}


