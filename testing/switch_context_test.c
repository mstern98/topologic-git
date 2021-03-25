// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"
#include "../include/test.h"

void init(struct graph **);
void setup_start_set(struct graph *graph);
void setup_non_start_set(struct graph *graph);
void test_run_switch(struct graph *);
void cleanup(struct graph *);

#define MAXIMUM 40
#define DEFAULT_BUFFER 64

void request_nil(void *args) {
	printf("RQST\n");
}

int edgeFunction(int id, void *args, void* glbl, const void* const edge_vars_a, const void *const edge_vars_b)
{
	int x = *(int *)(args);
	int y = *(int *)(((char *) args) + sizeof(int));
	return ((x * y) / 2) << 2;
}

void vertexFunction(int id, struct graph *graph, struct vertex_result* args, void* glbl, void* edge_vars)
{
	struct vertex_result *res = (struct vertex_result *) args;
	fprintf(stderr, "FIRING: %p, %d\n", res, *(int *) res->edge_argv);
	*(int *) res->edge_argv += 1;
	//return res;
	struct request *request = create_request(GENERIC, NULL, request_nil);
	submit_request(graph, request);
}

int main()
{
	struct graph *graph;
	init(&graph);
	assert(graph != NULL);

	fprintf(stderr, "PREPARING TO TEST CONTEXT MECHANISM: \"SWITCH\" CONTEXT\n");
	setup_start_set(graph);

	test_run_switch(graph);

	cleanup(graph);
	fprintf(stderr, "GRAPH CONTEXT \"SWITCH\" PASSED\n");
	return 0;
}

void cleanup(struct graph *graph)
{
	assert(graph != NULL);
	int i = 0;

	for (i = 0; i < MAXIMUM; i++)
	{
		struct vertex *v = (struct vertex *)find(graph->vertices, i);
		struct vertex *v2 = (struct vertex *)find(graph->vertices, ((i + 1) >= MAXIMUM ? 0 : i + 1));
		assert(v != NULL);
		assert(v2 != NULL);
		struct edge *e = (struct edge *)find(v->edge_tree, v2->id);
		if (e->glbl)
		{
			free(e->glbl);
			e->glbl = NULL;
		}
		assert(remove_edge(v, v2) == 0);
	}
	
	for (i = 0; i < MAXIMUM; i++)
	{
		struct vertex *v = (struct vertex *)find(graph->vertices, i);
		if (v == NULL)
			continue;
		if (v->glbl)
		{
			free(v->glbl);
			v->glbl = NULL;
		}
		remove_vertex(graph, v);
	}
	destroy_graph(graph);
	graph = NULL;
}

void init(struct graph **graph)
{
	*graph = graph_init(50, START_STOP, 10, VERTICES | EDGES | FUNCTIONS | GLOBALS, SWITCH, CONTINUE, IGNORE_FAIL_REQUEST);
	assert(*graph != NULL);

	//Setting up graphs and whatnot
	int i = 0;
	for (i = 0; i < MAXIMUM; i++)
	{
		int id = i;
		void (*f)(int, struct graph *, struct vertex_result*, void* glbl, void* edge_vars) = vertexFunction;
		//struct vertex_result *(*f)(void *) = vertexFunction;
		void *glbl = NULL;
		struct vertex_request *vert_req = (struct vertex_request *) malloc(sizeof(struct vertex_request));
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
		struct edge_request *edge_req = (struct edge_request *) malloc(sizeof(struct edge_request));
		assert(edge_req != NULL);
		edge_req->a = verts[i];
		edge_req->b = ((i + 1) >= MAXIMUM ? verts[0] : verts[i + 1]);
		edge_req->f = &edgeFunction;
		edge_req->glbl = NULL;
		struct request *req = create_request(CREAT_EDGE, edge_req, NULL);
		assert(submit_request(*graph, req) == 0);
	}
    struct edge_request *edge_req = (struct edge_request *) malloc(sizeof(struct edge_request));
    edge_req->a = verts[1];
    edge_req->b = verts[0];
    edge_req->f = &edgeFunction;
    edge_req->glbl = NULL;
    struct request *req = create_request(CREAT_EDGE, edge_req, NULL);
    assert(submit_request(*graph, req) == 0);

	assert(process_requests(*graph) == 0);
}

void setup_start_set(struct graph *graph)
{
	assert(graph != NULL);

	int ids[3] = {0,1,2};
	assert(start_set(graph, &ids[0], 1) == 0);
	ids[0] = 3;
	assert(start_set(graph, &ids[0], 1)==0);
	ids[0] = 0;
	assert(start_set(graph, ids, 3)==0);
	fprintf(stderr, "START SET TESTS COMPLETED\n");
}

void test_run_switch(struct graph *graph)
{
	assert(graph != NULL);

	struct vertex_result **vertex_args = (struct vertex_result **) malloc(sizeof(struct vertex_result *)*3);
	assert(vertex_args != NULL);

	int i = 0;
	for(i=0; i<3; i++){
		int edge_args[2] = {i + 1, i + 4};
		void *edge = malloc(sizeof(int) * 2);
		memcpy(edge, &edge_args[0], sizeof(int));
		memcpy(((char *) edge) + sizeof(int), &edge_args[1], sizeof(int));
		void *vertex = malloc(sizeof(int));
		*(int *)vertex = 10;

		struct vertex_result *v = (struct vertex_result *) malloc(sizeof(struct vertex_result));
		v->edge_argv = edge;
		v->edge_size = sizeof(int) * 2;
		v->vertex_argv = vertex;
		v->vertex_size = sizeof(int);
		vertex_args[i] = v;
	}

	assert(run(graph, vertex_args) == 0);
	//print_graph(graph);
	//for(i = 0; i<MAXIMUM;i++){
	//	free(vertex_args[i]);
	//}
	//free(vertex_args);
	//free(vertex_args);
	fprintf(stderr, "\"SWITCH\" RUN FINISHED\n");
}
