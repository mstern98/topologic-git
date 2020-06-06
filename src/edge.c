#include "../include/topologic.h"

struct edge *create_edge(struct vertex *a, struct vertex *b, int (*f)(int, void *), int argc, int glblc, void *glbl) {
    if(!a || !b){ return NULL; }
    if(!f){ return NULL; }

    pthread_mutex_lock(&a->lock); 
    void *exists = find(a->edge_tree, b->id);
    if (exists != NULL) {
        pthread_mutex_lock(&a->lock); 
        return NULL; 
    }

    struct edge* edge = malloc(sizeof(struct edge));
    if(!edge){ 
        pthread_mutex_lock(&a->lock); 
        return NULL;
    }
    edge->a = a;
    edge->b = b;

    edge->glblc = glblc;
    edge->glbl = glbl;

    edge->a_varc = a->edge_sharedc;
    edge->a_vars = a->edge_shared;

    edge->f = f;
    //MAKE UNIQUE
    edge->id = b->id;

    if(insert(a->edge_tree, (void **)(&edge), edge->id) < 0){
        free(edge);
        edge = NULL;
        pthread_mutex_unlock(&a->lock); 
        return NULL;
    }
    pthread_mutex_unlock(&a->lock); 
    return edge;
}

struct edge **create_bi_edge(struct vertex *a, struct vertex *b, int (*f)(int, void *), int argc, int glblc, void *glbl) {
    if(!a || !b || !f) return NULL;
    struct edge** bi_edge = malloc(sizeof(struct edge) * 2);
    if(bi_edge==NULL){ return NULL;}
    
    bi_edge[0] = create_edge(a, b, f, argc, glblc, glbl);
    if (bi_edge[0] == NULL) {
        free(bi_edge);
        bi_edge = NULL;
        return NULL;
    }
    bi_edge[1] = create_edge(a, b, f, argc, glblc, glbl);
    if (!bi_edge[1]) {
        remove_edge(a, b);
        free(bi_edge[0]);
        bi_edge[0] = NULL;
        free(bi_edge);
        bi_edge = NULL;
        return NULL;
    }

    return bi_edge;
}

int remove_edge(struct vertex *a, struct vertex *b) {
    if (!a || !b) return -1;
    pthread_mutex_lock(&a->lock); 
    void *data = remove_ID(a->edge_tree, b->id);
    if (!data) return -1;
    struct edge *edge = (struct edge *) data;
    edge->a_varc = 0;
    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    free(edge->glbl);
    edge->glbl = NULL;
    edge->glblc = 0;
    
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
    edge->a_varc = 0;
    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;
    free(edge->glbl);
    edge->glbl = NULL;
    edge->glblc = 0;
    
    free(edge);
    edge = NULL;
    pthread_mutex_unlock(&a->lock); 
    return 0;
}

int remove_bi_edge(struct vertex *a, struct vertex *b) {
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = remove_edge(a, b)) < 0) ret = -2;
    if ((b_ret = remove_edge(b, a)) < 0 && a < 0) ret = -1;
    if (b_ret < 0 && a_ret == 0) ret = -3;

    return ret;
}

int modify_edge(struct vertex *a, struct vertex *b, int (*f)(int, void *), int argc, int glblc, void *glbl) {
    if (!a || !b) return -1;
    pthread_mutex_lock(&a->lock); 
    struct edge *edge = (struct edge *) find(a->edge_tree, b->id);
    if (!edge) return -1;
    if (f) {
        edge->f = f;
        edge->argc = argc;
    }
    if (glbl) {
        free(edge->glbl);
        edge->glbl = glbl;
        edge->glblc = glblc;
    }
    pthread_mutex_unlock(&a->lock); 
    return 0;
}

int modify_bi_edge(struct vertex *a, struct vertex *b, int (*f)(int, void *), int argc, int glblc, void *glbl) {
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = modify_edge(a, b, f, argc, glblc, glbl)) < 0) ret = -2;
    if ((b_ret = modify_edge(b, a, f, argc, glblc, glbl)) < 0 && a < 0) ret = -1;
    if (b_ret < 0 && a_ret == 0) ret = -3;

    return ret;
}


