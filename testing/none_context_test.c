// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"
#include "../include/test.h"

void init(struct graph **);
void setup_start_set(struct graph *graph);
void setup_non_start_set(struct graph *graph);
void test_run_none(struct graph *);
void cleanup(struct graph *);

#define MAXIMUM 24
#define DEFAULT_BUFFER 64

int edgeFunction(void *args, void *glbl, const void *const edge_vars_a, const void *const edge_vars_b)
{
	int x = *(int *)(args);
	int y = *(int *)(args + sizeof(int));
	fprintf(stderr, " edge : %d\n", ((x * y) / 2) << 2);
	return ((x * y) / 2) << 2;
}

void vertexFunction(struct graph *graph, struct vertex_result* args, void* glbl, void* edge_vars)
{
	struct vertex_result *res = (struct vertex_result *)args;
	fprintf(stderr, "FIRING: %p, %d\n", res, *(int *) res->vertex_argv);
	*(int *) res->edge_argv += 1;
	//return res;
}

int main()
{
	struct graph *graph;
	init(&graph);
	assert(graph != NULL);

	fprintf(stderr, "PREPARING TO TEST CONTEXT MECHANISM: \"NONE\" CONTEXT\n");
	setup_start_set(graph);

	test_run_none(graph);

	cleanup(graph);
	fprintf(stderr, "GRAPH CONTEXT \"NONE\" PASSED\n");
	return 0;
}

void cleanup(struct graph *graph)
{
	assert(graph != NULL);
	destroy_graph(graph);
	graph = NULL;
}

void init(struct graph **graph)
{
	*graph = graph_init(100, START_STOP, 1, VERTICES | EDGES | FUNCTIONS | GLOBALS, NONE, CONTINUE);
	assert(*graph != NULL);

	//Setting up graphs and whatnot
	//
	int i = 0;
	for (i = 0; i < MAXIMUM; i++)
	{
		int id = i;
		void (*f)(struct graph *, struct vertex_result*, void* glbl, void* edge_vars) = vertexFunction;
		//struct vertex_result *(*f)(void *) = vertexFunction;
		void *glbl = NULL;
		struct vertex_request *vert_req = malloc(sizeof(struct vertex_request));
		vert_req->graph = *graph;
		vert_req->id = id;
		vert_req->f = f;
		vert_req->glbl = glbl;
		struct request *req = create_request(CREAT_VERTEX, vert_req, NULL);
		assert(submit_request(*graph, req) == 0);
	}
	assert(process_requests(*graph) == 0);

	struct vertex *verts[MAXIMUM];
	for (i = 0; i < MAXIMUM; i++)
	{
		verts[i] = (struct vertex *)(find((*graph)->vertices, i));
		assert(verts[i] != NULL);
	}
	for (i = 0; i < MAXIMUM; i++)
	{
		struct edge_request *edge_req = malloc(sizeof(struct edge_request));
		assert(edge_req != NULL);
		edge_req->a = verts[i];
		edge_req->b = ((i + 1) >= MAXIMUM ? verts[0] : verts[i + 1]);
		edge_req->f = &edgeFunction;
		edge_req->glbl = NULL;
		struct request *req = create_request(CREAT_EDGE, edge_req, NULL);
		assert(submit_request(*graph, req) == 0);
	}
	assert(process_requests(*graph) == 0);
}

void setup_start_set(struct graph *graph)
{
	assert(graph != NULL);

	int ids[2] = {0, 4};
	assert(start_set(graph, &ids[0], 1) == 0);
	ids[0] = 3;
	assert(start_set(graph, &ids[0], 1)==0);
	ids[0] = 9;
	assert(start_set(graph, ids, 2)==0);
	fprintf(stderr, "START SET TESTS COMPLETED\n");
}

void test_run_none(struct graph *graph)
{
	assert(graph != NULL);

	struct vertex_result **vertex_args = malloc(sizeof(struct vertex_result *) * 2);
	assert(vertex_args != NULL);

	int i = 0;
	for (i = 0; i < 2; ++i) {
		int edge_args[2] = {i + 1, i + 4};
		void *edge = malloc(sizeof(int) * 2);
		memcpy(edge, &edge_args[0], sizeof(int));
		memcpy(edge + sizeof(int), &edge_args[1], sizeof(int));
		void *vertex = malloc(sizeof(int));
		*(int *)vertex = 10;
		struct vertex_result *v = malloc(sizeof(struct vertex_result));
		v->edge_argv = edge;
		v->edge_size = 0;
		v->vertex_argv = vertex;
		v->vertex_size = 0;
		vertex_args[i] = v;
	}

	assert(run(graph, vertex_args) == 0);
	//print_graph(graph);
	//for(i = 0; i<MAXIMUM;i++){
	//	free(vertex_args[i]);
	//}
	//free(vertex_args);
	//free(vertex_args);
	fprintf(stderr, "\"NONE\" RUN FINISHED\n");
}
