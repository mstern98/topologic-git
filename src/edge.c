#include "../include/topologic.h"

struct edge *create_edge(struct graph *graph, int id, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    if(!graph){ return NULL; }
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
    edge->glbl = glbl;
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

struct edge **create_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    
    if(!graph || !a || !b || !f) return NULL;
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

int remove_edge(struct graph *graph, struct vertex *a, struct vertex *b, int id) {
    return 0;
}

int remove_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, int id) {
    return 0;
}

int modify_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc,int glblc, void *glbl) {
    return 0;
}

int modify_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc,int glblc, void *glbl) {
    return 0;
}


