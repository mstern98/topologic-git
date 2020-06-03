#include "../include/topologic.h"

struct edge *create_edge(int id, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    if(!a || !b){ return NULL; }
    if(!f){ return NULL; }

    struct stack *preorder = preorder(a->edge_tree);
    if (!preorder) {
        struct stack_node *node = pop(preorder);
        while (node != NULL) {
            if (((struct vertex *) node->data)->id == b->id)
                return NULL;
        }
    }

    struct edge* edge = malloc(sizeof(struct edge));
    if(!edge){ return NULL };
    edge->a = a;
    edge->b = b;

    edge->glblc = glblc;
    if (glblc > 0) {
        edge->glbl = malloc(sizeof(void *) * glblc);
        memcpy(edge->glbl, glbl, sizeof(void *) * glblc);
    }

    edge->a_varc = a->edge_sharedc;
    edge->a_vars = a->edge_shared;

    edge->f = f;
    //MAKE UNIQUE
    edge->id = 0;

    if(insert(a->edge_tree, edge, edge->id) < 0){
        free(edge);
        edge = NULL;
        return NULL;
    }
    
    return edge;
}

struct edge **create_bi_edge(struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    if(!a || !b || !f) return NULL;
    struct edge** bi_edge = malloc(sizeof(struct edge) * 2);
    if(bi_edge==NULL){ return NULL;}
    
    bi_edge[0] = create_edge(graph, a, b, f, argc, glblc, glbl);
    if (bi_edge[0] == NULL) {
        free(bi_edge);
        bi_edge = NULL;
        return NULL;
    }
    bi_edge[1] = malloc(sizeof(struct edge));
    if (!bi_edge[1]) {
        free(bi_edge[0]);
        bi_edge[0] = NULL;
        free(bi_edge);
        bi_edge = NULL;
        return NULL;
    }
    memcpy(bi_edge[1], bi_edge[0], sizeof(struct edge));
    if(insert(b->edge_tree, bi_edge[1], bi_edge[1]->id) < 0){
        remove_ID(a->edge_tree, bi_edge[0]->id);
        free(bi_edge[0]);
        bi_edge[0] = NULL;
        free(bi_edge[1]);
        bi_edge[1] = NULL;
        free(bi_edge);
        bi_edge = NULL;
        return NULL;
    }

    return bi_edge;
}

int remove_edge(struct vertex *a, struct vertex *b, int id) {
    void *data = remove_ID(a->edge_tree, id);
    if (!data) return -1;
    struct edge *edge = (struct edge *) data;
    edge->a_varc = 0;
    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    if (edge->glblc > 1) {
        int i;
        for (i = 0; i < edge->glblc; i++) {
            free(edge->glbl[i]);
            edge->glbl[i] = NULL;
        }
    } else if (edge->glbl == 1) {
        free(edge->glbl);
        edge->glbl = NULL;
    }
    edge->glblc = 0;
    
    free(edge);
    edge = NULL;
    return 0;
}

int remove_bi_edge(struct vertex *a, struct vertex *b, int id) {
    int ret = 0, a = 0, b = 0;
    if ((a = remove_edge(a, b, id)) < 0) ret = -2;
    if ((b = remove_edge(b, a, id)) < 0 && a < 0) ret = -1;
    if (b < 0 && a == 0) ret = -3;

    return ret;
}

int modify_edge(struct vertex *a, struct vertex *b, int id, void (*f)(void *), int argc, int glblc, void *glbl) {
    if (!a || !b) return -1;
    struct edge *edge = (struct edge *) find(a->edge_tree, id);
    if (!edge) return -1;
    if (f) {
        edge->f = f;
        edge->argc = argc;
    }
    if (glbl) {
        if (edge->glblc > 1) {
            int i;
            for (i = 0; i < edge->glblc; i++) {
                free(edge->glbl[i]);
                edge->glbl[i] = NULL;
            }
        } else if (edge->glbl == 1) {
            free(edge->glbl);
            edge->glbl = NULL;
        }
        edge->glblc = glblc;
        if (glblc > 0) {
            edge->glbl = malloc(sizeof(void *) * glblc);
            memcpy(edge->glbl, glbl, sizeof(void *) * glblc);
        }
    }
    return 0;
}

int modify_bi_edge(struct vertex *a, struct vertex *b, int id, void (*f)(void *), int argc, int glblc, void *glbl) {
    int ret = 0, a = 0, b = 0;
    if ((a = modify_edge(a, b, id, f, argc, glblc, glbl)) < 0) ret = -2;
    if ((b = modify_edge(b, a, id, f, argc, glblc, glbl)) < 0 && a < 0) ret = -1;
    if (b < 0 && a == 0) ret = -3;

    return ret;
}


