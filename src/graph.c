// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct graph *graph_init(int max_state_changes, int snapshot_timestamp, int max_loop, unsigned int lvl_verbose, enum CONTEXT context, enum MEM_OPTION mem_option)
{
	topologic_debug("%s;max_state_chages %d;snapshot_timestamp %d;max_loop %d;lvl_verbose %d;context %d;mem_option %d", "graph_init", max_state_changes, snapshot_timestamp, max_loop, lvl_verbose, context, mem_option);
	struct graph *graph = (struct graph *)malloc(sizeof(struct graph));
	if (!graph)
	{
		topologic_debug("%s;%s;%p", "graph_init", "failed to malloc", (void *) NULL);
		return NULL;
	}
	graph->max_state_changes = max_state_changes;
	graph->max_loop = max_loop;
	graph->snapshot_timestamp = snapshot_timestamp;
	graph->lvl_verbose = lvl_verbose;
	graph->context = context;
	graph->state_count = 0;
	graph->mem_option = mem_option;
	graph->pause = 0;
	graph->red_locked = 1;
	graph->black_locked = 1;
	graph->num_vertices = 0;

	if (pthread_mutex_init(&graph->lock, NULL) < 0)
	{
		free(graph);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create lock", (void *) NULL);
		return NULL;
	}

	if (pthread_cond_init(&graph->pause_cond, NULL) < 0)
	{
		pthread_mutex_destroy(&graph->lock);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create pause cond", (void *) NULL);
		free(graph);
		return NULL;
	}

	if (context != SINGLE)
	{
		if (pthread_mutex_init(&graph->color_lock, NULL) < 0)
		{
			pthread_mutex_destroy(&graph->lock);
			pthread_cond_destroy(&graph->pause_cond);
			topologic_debug("%s;%s;%p", "graph_init", "failed to create color lock", (void *) NULL);
			free(graph);
			return NULL;
		}
		if (pthread_cond_init(&graph->red_fire, NULL) < 0)
		{
			pthread_mutex_destroy(&graph->lock);
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->pause_cond);
			topologic_debug("%s;%s;%p", "graph_init", "failed to create red fire cond", (void *) NULL);
			free(graph);
			return NULL;
		}
		if (pthread_cond_init(&graph->black_fire, NULL) < 0)
		{
			pthread_mutex_destroy(&graph->lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->pause_cond);
			topologic_debug("%s;%s;%p", "graph_init", "failed to create black fire cond", (void *) NULL);
			free(graph);
			return NULL;
		}
	}

	graph->vertices = init_avl();
	if (!graph->vertices)
	{
		pthread_mutex_destroy(&graph->lock);
		pthread_cond_destroy(&graph->pause_cond);
		if (context != SINGLE)
		{
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_cond_destroy(&graph->black_fire);
		}
		free(graph);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create avl", (void *) NULL);
		return NULL;
	}

	graph->modify = init_stack();
	if (!graph->modify)
	{
		destroy_avl(graph->vertices);
		pthread_mutex_destroy(&graph->lock);
		pthread_cond_destroy(&graph->pause_cond);
		if (context != SINGLE)
		{
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_cond_destroy(&graph->black_fire);
		}
		free(graph);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create stack", (void *) NULL);
		return NULL;
	}
	graph->remove_edges = init_stack();
	if (!graph->remove_edges)
	{
		destroy_avl(graph->vertices);
		destroy_stack(graph->modify);
		pthread_mutex_destroy(&graph->lock);
		pthread_cond_destroy(&graph->pause_cond);
		if (context != SINGLE)
		{
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_cond_destroy(&graph->black_fire);
		}
		free(graph);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create stack", (void *) NULL);
		return NULL;
	}
	graph->remove_vertices = init_stack();
	if (!graph->remove_vertices)
	{
		destroy_avl(graph->vertices);
		destroy_stack(graph->modify);
		destroy_stack(graph->remove_edges);
		pthread_mutex_destroy(&graph->lock);
		pthread_cond_destroy(&graph->pause_cond);
		if (context != SINGLE)
		{
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_cond_destroy(&graph->black_fire);
		}
		topologic_debug("%s;%s;%p", "graph_init", "failed to create stack", (void *) NULL);
		free(graph);
		return NULL;
	}
	graph->start = init_stack();
	if (!graph->start)
	{
		destroy_avl(graph->vertices);
		destroy_stack(graph->modify);
		destroy_stack(graph->remove_edges);
		destroy_stack(graph->remove_vertices);
		pthread_mutex_destroy(&graph->lock);
		pthread_cond_destroy(&graph->pause_cond);
		if (context != SINGLE)
		{
			pthread_mutex_destroy(&graph->color_lock);
			pthread_cond_destroy(&graph->red_fire);
			pthread_cond_destroy(&graph->black_fire);
		}
		free(graph);
		topologic_debug("%s;%s;%p", "graph_init", "failed to create stack", (void *) NULL);
		return NULL;
	}

	graph->previous_color = RED;
	graph->state = RED;
	graph->red_vertex_count = 0;
	graph->black_vertex_count = 0;
	graph->print_flag = 0;
	topologic_debug("%s;%s;%p", "graph_init", "success", graph);
	return graph;
}

void destroy_graph_stack(struct stack *stack)
{
	if (!stack)
		return;
	struct request *request = NULL;
	while ((request = (struct request *)pop(stack)) != NULL)
	{
		destroy_request(request);
	}
	destroy_stack(stack);
	stack = NULL;
}

void destroy_graph_avl(struct graph *graph, struct AVLTree *tree)
{
	if (!tree)
		return;

	struct stack *tree_stack = init_stack();
	if (!tree_stack)
		return;
	preorder(tree, tree_stack);
	struct vertex *vertex = NULL;
	while ((vertex = (struct vertex *)pop(tree_stack)) != NULL)
	{
		remove_vertex(graph, vertex);
	}
	destroy_stack(tree_stack);
	tree_stack = NULL;
	destroy_avl(tree);
	tree = NULL;
}

int destroy_graph(struct graph *graph)
{
	topologic_debug("%s;graph %p", "destroy_graph", graph);
	if (!graph)
	{
		topologic_debug("%s;%s;%d", "destroy_graph", "invalid args", -1);
		return -1;
	}
	graph->state = TERMINATE;

	if (graph->red_vertex_count >= 0)
	{
		if (graph->context != SINGLE)
		{
			graph->red_locked = 0;
			while (graph->red_vertex_count > 0)
			{
			}
		}
	}

	if (graph->black_vertex_count >= 0)
	{
		if (graph->context != SINGLE)
		{
			graph->black_locked = 0;
			while (graph->black_vertex_count > 0)
			{
			}
		}
	}

	destroy_graph_avl(graph, graph->vertices);
	graph->vertices = NULL;
	destroy_graph_stack(graph->start);
	graph->start = NULL;
	destroy_graph_stack(graph->modify);
	graph->modify = NULL;
	destroy_graph_stack(graph->remove_edges);
	graph->remove_edges = NULL;
	destroy_graph_stack(graph->remove_vertices);
	graph->remove_vertices = NULL;

	pthread_mutex_destroy(&graph->lock);
	pthread_cond_destroy(&graph->pause_cond);
	graph->red_locked = 0;
	graph->black_locked = 0;

	free(graph);
	graph = NULL;
	topologic_debug("%s;%s;%d", "destroy_graph", "finished", 0);
	return 0;
}
