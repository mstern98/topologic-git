// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct edge *create_edge(struct graph* graph, struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl)
{
    if (!a || !b)
    {
        return NULL;
    }
    if (!f)
    {
        return NULL;
    }
		if(!graph) return NULL;

		enum CONTEXT context = graph->context;

		if(context!=SINGLE)
			pthread_mutex_lock(&a->lock);
		void *exists = find(a->edge_tree, b->id);
    if (exists)
    {
        if(context!=SINGLE){pthread_mutex_unlock(&a->lock);}
        return NULL;
    }

    struct edge *edge = malloc(sizeof(struct edge));
    if (!edge)
    {
        if(context!=SINGLE){pthread_mutex_unlock(&a->lock);}
        return NULL;
    }
    edge->edge_type = EDGE;
    edge->a = a;
    edge->b = b;
    edge->bi_edge = NULL;

    edge->glbl = glbl;

    edge->a_vars = a->shared->edge_data;

    edge->f = f;
    //MAKE UNIQUE
    edge->id = b->id;

    if (insert(a->edge_tree, edge, edge->id) < 0)
    {
        edge->a = NULL;
        edge->b = NULL;
        edge->a_vars = NULL;
        edge->f = NULL;
        edge->glbl = NULL;
        edge->id = 0;
        free(edge);
        edge = NULL;
        if(context!=SINGLE){pthread_mutex_unlock(&a->lock);}
        return NULL;
    }

    if(context!=SINGLE){pthread_mutex_lock(&b->lock);}
    if (insert(b->joining_vertices, a, a->id) < 0)
    {
        remove_ID(a->edge_tree, edge->id);
        edge->a = NULL;
        edge->b = NULL;
        edge->a_vars = NULL;
        edge->f = NULL;
        edge->glbl = NULL;
        edge->id = 0;
        free(edge);
        edge = NULL;
       if(context!=SINGLE){
				 pthread_mutex_unlock(&b->lock);
         pthread_mutex_unlock(&a->lock);
			 }
    }

		if(context!=SINGLE){
			pthread_mutex_unlock(&b->lock);
			pthread_mutex_unlock(&a->lock);
		}
    return edge;
}

int create_bi_edge(struct graph* graph, struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl, struct edge **edge_a_to_b, struct edge **edge_b_to_a)
{
		if(!graph){ return -1;}
	  if (!a || !b || !f || a == b)
        return -1;
    struct edge *a_to_b, *b_to_a;
		enum CONTEXT context = graph->context;

    a_to_b = create_edge(graph,a, b, f, glbl);
    if (!a_to_b)
    {
        return -1;
    }
    a_to_b->edge_type = BI_EDGE;
		
		if(context!=SINGLE){
    if (pthread_mutex_init(&a_to_b->bi_edge_lock, NULL) < 0)
    {
        remove_edge(graph,a, b);
        a_to_b = NULL;
        return -1;
    }
		}

    b_to_a = create_edge(graph,b, a, f, glbl);
    if (!b_to_a)
    {
        remove_edge(graph,a, b);
        free(a_to_b);
        a_to_b = NULL;
        return -1;
    }
    b_to_a->edge_type = BI_EDGE;
    
		b_to_a->bi_edge_lock = a_to_b->bi_edge_lock;

    a_to_b->bi_edge = b_to_a;
    b_to_a->bi_edge = a_to_b;

    if (edge_a_to_b)
        *edge_a_to_b = a_to_b;
    if (edge_b_to_a)
        *edge_b_to_a = b_to_a;
    return 0;
}

int remove_edge(struct graph* graph, struct vertex *a, struct vertex *b)
{
    if (!a || !b)
        return -1;
		if(!graph) return -1;
    remove_ID(b->joining_vertices, a->id);

    if(graph->context!=SINGLE){pthread_mutex_lock(&a->lock);}
    void *data = remove_ID(a->edge_tree, b->id);
    if (!data)
    {
        if(graph->context!=SINGLE){pthread_mutex_unlock(&a->lock);}
        return -1;
    }

    struct edge *edge = (struct edge *)data;
    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    edge->glbl = NULL;

    if (edge->edge_type == BI_EDGE)
    {
        if(graph->context!=SINGLE){pthread_mutex_destroy(&edge->bi_edge_lock);}
        edge->bi_edge->bi_edge = NULL;
        edge->bi_edge->edge_type = EDGE;
    }
    edge->bi_edge = NULL;

    free(edge);
    edge = NULL;
    if(graph->context!=SINGLE){pthread_mutex_unlock(&a->lock);}
    return 0;
}

int remove_edge_id(struct graph* graph, struct vertex *a, int id)
{
    if (!a)
        return -1;
		if(!graph)
			  return -1;
    
		if(graph->context!=SINGLE){pthread_mutex_lock(&a->lock);}
    void *data = remove_ID(a->edge_tree, id);
    if (!data)
        return -1;
    struct edge *edge = (struct edge *)data;

    remove_ID(edge->b->joining_vertices, a->id);

    edge->a_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;
    edge->glbl = NULL;

    if (edge->edge_type == BI_EDGE)
    {
        if(graph->context!=SINGLE){pthread_mutex_destroy(&edge->bi_edge_lock);}
        edge->bi_edge->bi_edge = NULL;
        edge->bi_edge->edge_type = EDGE;
    }
    edge->bi_edge = NULL;

    free(edge);
    edge = NULL;
    if(graph->context!=SINGLE){pthread_mutex_unlock(&a->lock);}
    return 0;
}

int remove_bi_edge(struct graph* graph, struct vertex *a, struct vertex *b)
{
	  if(!graph){
				return -1;
		}
    if (!a || !b || a == b)
        return -1;
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = remove_edge(graph,a, b)) < 0)
        ret = -2;
    if ((b_ret = remove_edge(graph,b, a)) < 0 && a < 0)
        ret = -1;
    if (b_ret < 0 && a_ret == 0)
        ret = -3;

    return ret;
}

int modify_edge(struct graph* graph, struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl)
{
    if (!a || !b)
        return -1;
    if(graph->context!=SINGLE){pthread_mutex_lock(&a->lock);}
    struct edge *edge = (struct edge *)find(a->edge_tree, b->id);
    if (!edge)
    {
        if(graph->context!=SINGLE){pthread_mutex_unlock(&a->lock);}
        return -1;
    }
    if (f)
    {
        edge->f = f;
    }
    if (glbl)
    {
        edge->glbl = glbl;
    }
    if(graph->context!=SINGLE){pthread_mutex_unlock(&a->lock);}
    return 0;
}

int modify_bi_edge(struct graph* graph, struct vertex *a, struct vertex *b, int (*f)(void *), void *glbl)
{
    if (!a || !b || a == b)
        return -1;
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = modify_edge(graph,a, b, f, glbl)) < 0)
        ret = -2;
    if ((b_ret = modify_edge(graph,b, a, f, glbl)) < 0 && a < 0)
        ret = -1;
    if (b_ret < 0 && a_ret == 0)
        ret = -3;

    return ret;
}
