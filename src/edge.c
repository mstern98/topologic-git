#include "../include/topologic.h"

struct edge *create_edge(struct graph *graph, int id, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    if(!graph){ return NULL; }
    if(!a || !b){ return NULL; }
    if(!f){ return NULL; }

    struct edge* edge = malloc(sizeof(struct edge));
    if(!edge){ return NULL };
    edge->a = malloc(sizeof(struct vertex));
    if(!edge->a){
        free(edge); return NULL;
    }
    edge->b = malloc(sizeof(struct vertex));
    if(!edge->b){ free(edge-> a); free(edge); return NULL;}
    
    if(memcpy(edge->a, a, sizeof(struct vertex))==NULL){ 
        free(edge->a); free(edge->b); free(edge);
        return NULL; 
    }
    if(memcpy(edge->b, b, sizeof(struct vertex))==NULL){ 
        free(edge->a); free(edge->b); free(edge);
        return NULL; 
    }

    edge->glbl = malloc(sizeof(void *));
    if(!edge->glbl){
        free(edge->a); free(edge->b); free(edge);
        return NULL;
    }
    if(memcpy(edgle->glbl, glbl, sizeof(void *))==NULL){
        free(edge->glbl); free(edge->a); free(edge->b); free(edge);
        return NULL;
    }

    edge->f = f;
    edge->glblc = glblc;
    if(find(a->edge_tree, id)!=NULL || find(b->edge_tree, id)!=NULL){
        free(edge->glbl); free(edge->a); free(edge->b); free(edge);
        return NULL;
    }
    struct AVLNode* newNode = malloc(sizeof(struct AVLNode));
    if(!newNode){
        free(edge->glbl); free(edge->a); free(edge->b); free(edge);
        return NULL;
    }
    newNode->id = id;
    newNode->height = 0;
    newNode->left = newNode->right = NULL;
    newNode->data = glbl; //TODO: Copy edge data into a buffer

    if(insert_node(edge->a->edge_tree->root, newNode)==NULL){
        free(newNode); free(edge->glbl); free(edge->a); free(edge->b); free(edge);
        return NULL;
    }
     if(insert_node(edge->b->edge_tree->root, newNode)==NULL){
        free(newNode); free(edge->glbl); free(edge->a); free(edge->b); free(edge);
        return NULL;
    }


    edge->id = id;
    return edge;
}

struct edge **create_bi_edge(struct graph *graph, struct vertex *a, struct vertex *b, void (*f)(void *), int argc, int glblc, void *glbl) {
    
    if(!graph || !a || !b || !f) return NULL;
    struct edge** bi_edge = malloc(sizeof(struct edge) * 2);
    if(bi_edge==NULL){ return NULL;}
    

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


