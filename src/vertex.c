// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct vertex *create_vertex(struct graph *graph, void (*f)(int, struct graph *, struct vertex_result *, void *, void *), int id, void *glbl)
{
	topologic_debug("%s;graph %p;f %p;id %d; glbl %p", "create_vertex", graph, f, id, glbl);
	if (!graph || !f)
	{
		topologic_debug("%s;%s;%p", "create_vertex", "invalid", (void *) NULL);
		return NULL;
	}
	enum CONTEXT context = graph->context;
	if (context != SINGLE)
		pthread_mutex_lock(&graph->lock);

	struct vertex *vertex = (struct vertex *)malloc(sizeof(struct vertex));
	if (!vertex)
	{
		topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
        goto unlock_graph;
	}
	vertex->context = context;

	vertex->shared = (union shared_edge *)malloc(sizeof(union shared_edge));
	if (!vertex->shared)
	{
		topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
        goto free_vertex;
	}
	vertex->shared->vertex_data = NULL;

	vertex->edge_tree = init_avl();
	if (!vertex->edge_tree)
	{
        topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
        goto free_shared;
	}

	vertex->joining_vertices = init_avl();
	if (!vertex->joining_vertices)
	{
		topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
        goto destroy_edge_tree;
	}

	if (insert(graph->vertices, vertex, id) < 0)
	{
		topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
        goto destroy_joining_vertices;
	}

	if (context != SINGLE)
	{
		if (pthread_mutex_init(&vertex->lock, NULL) < 0)
		{
			topologic_debug("%s;%s;%p", "create_vertex", "failed to initialize", (void *) NULL);
            goto destroy_joining_vertices;
		}
	}

	vertex->is_active = 0;
	vertex->f = f;
	vertex->id = id;
	vertex->glbl = glbl;
	if (context != SINGLE)
		pthread_mutex_unlock(&graph->lock);
	topologic_debug("%s;%s;%p", "create_vertex", "succeeded", vertex);
	return vertex;

destroy_joining_vertices:
	destroy_avl(vertex->joining_vertices);
	vertex->joining_vertices = NULL;
destroy_edge_tree:
    destroy_avl(vertex->edge_tree);
	vertex->edge_tree = NULL;
free_shared:
    free(vertex->shared);
    vertex->shared = NULL;
free_vertex:
    free(vertex);
	vertex = NULL;
unlock_graph:
    if (context != SINGLE)
		pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s", "create_vertex", "failed");
    return NULL;

}

int remove_vertex(struct graph *graph, struct vertex *vertex)
{
	topologic_debug("%s;graph %p;vertex %p", "remove_vertex", graph, vertex);
	if (!graph || !vertex)
	{
		topologic_debug("%s;%s;%d", "remove_vertex", "invalid graph or vertex", -1);
		return -1;
	}
	enum CONTEXT context = graph->context;
	if (context != SINGLE)
	{
		pthread_mutex_lock(&graph->lock);
		pthread_mutex_lock(&vertex->lock);
	}

	if (!remove_ID(graph->vertices, vertex->id))
	{
		if (context != SINGLE)
		{
			pthread_mutex_unlock(&vertex->lock);
			pthread_mutex_unlock(&graph->lock);
		}
		topologic_debug("%s;%s;%d", "remove_vertex", "vertex not in graph", -1);
		return -1;
	}

	struct stack *stack = init_stack();
	stackify(vertex->edge_tree, stack);
	vertex->edge_tree = NULL;
	struct edge *edge = NULL;
	while ((edge = (struct edge *)pop(stack)) != NULL)
	{
		if (context != SINGLE)
		{
			if (edge->edge_type == SELF_EDGE)
				pthread_mutex_unlock(&vertex->lock);
			pthread_mutex_lock(&(edge->b->lock));
		}
		remove_ID(edge->b->joining_vertices, vertex->id);
		if (context != SINGLE)
			pthread_mutex_unlock(&(edge->b->lock));
		if (edge->edge_type == BI_EDGE)
		{
			if (context != SINGLE)
				pthread_mutex_destroy(&edge->bi_edge_lock);
			edge->bi_edge->bi_edge = NULL;
			edge->bi_edge->edge_type = EDGE;
		}
		edge->bi_edge = NULL;
		edge->a = NULL;
		edge->b = NULL;
		edge->id = 0;
		edge->f = NULL;
		if (edge->edge_type != BI_EDGE && edge->glbl)
			free(edge->glbl);
		edge->glbl = NULL;
		free(edge);
		edge = NULL;
	}

	stackify(vertex->joining_vertices, stack);
	vertex->joining_vertices = NULL;
	struct vertex *joining_vertex = NULL;
	while ((joining_vertex = (struct vertex *)pop(stack)) != NULL)
		remove_edge_id(joining_vertex, vertex->id);

	destroy_stack(stack);
	stack = NULL;

	vertex->id = 0;
	if (vertex->shared->vertex_data)
		free(vertex->shared->vertex_data);
	vertex->shared->vertex_data = NULL;
	free(vertex->shared);
	vertex->shared = NULL;
	if (vertex->glbl)
		free(vertex->glbl);
	vertex->glbl = NULL;

	if (context != SINGLE)
	{
		pthread_mutex_unlock(&vertex->lock);
		pthread_mutex_destroy(&vertex->lock);
		pthread_mutex_unlock(&graph->lock);
	}

	free(vertex);
	vertex = NULL;
	topologic_debug("%s;%s;%d", "remove_vertex", "success", 0);
	return 0;
}

int remove_vertex_id(struct graph *graph, int id)
{
	topologic_debug("%s;graph %p;id %d", "remove_vertex_id", graph, id);
	if (!graph)
	{
		topologic_debug("%s;%s;%d", "remove_vertex_id", "invalid graph", -1);
		return -1;
	}
	if (graph->context != SINGLE)
		pthread_mutex_lock(&graph->lock);
	struct vertex *vertex = (struct vertex *)find(graph->vertices, id);
	if (graph->context != SINGLE)
		pthread_mutex_unlock(&graph->lock);
	if (!vertex)
	{
		topologic_debug("%s;%s;%d", "remove_vertex_id", "invalid vertex", -1);
		return -1;
	}
	return remove_vertex(graph, vertex);
}

int modify_vertex(struct vertex *vertex, void (*f)(int, struct graph *, struct vertex_result *, void *, void *), void *glbl)
{
	topologic_debug("%s;vertex %p;f %p;glbl %p", "modify_vertex", vertex, f, glbl);
	if (!vertex){
		topologic_debug("%s;%s;%d", "modify_vertex", "invalid vertex", -1);
		return -1;}

	if (vertex->context != SINGLE)
		pthread_mutex_lock(&vertex->lock);

	if (f != NULL)
		vertex->f = f;
	if (glbl != NULL)
	{
		if (vertex->glbl)
			free(vertex->glbl);
		vertex->glbl = glbl;
	}

	if (vertex->context != SINGLE)
		pthread_mutex_unlock(&vertex->lock);
	topologic_debug("%s;%s;%d", "modify_vertex", "success", 0);
	return 0;
}

int modify_shared_edge_vars(struct vertex *vertex, void *edge_vars)
{
	topologic_debug("%s;vertex %p;edge_vars %p", "modify_shared_edge_vars", vertex, edge_vars);
	if (!vertex){
		topologic_debug("%s;%s;%d", "modify_shared_edge_vars", "invalid vertex", -1);
		return -1;}
	if (vertex->context != SINGLE)
		pthread_mutex_lock(&vertex->lock);

	if (vertex->shared->vertex_data)
		free(vertex->shared->vertex_data);
	vertex->shared->vertex_data = edge_vars;
	if (vertex->context != SINGLE)
		pthread_mutex_unlock(&vertex->lock);
	topologic_debug("%s;%s;%d", "modify_shared_edge_vars", "success", 0);
	return 0;
}
