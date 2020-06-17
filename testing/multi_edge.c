// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"
#include "../include/test.h"

void init(struct graph**);
void setupVertex(struct graph*);
void setupEdges(struct graph*);
void runTest(struct graph*);
void cleanup(struct graph*);
#define MAXIMUM 6


int edgeFunction(void *args)
{
	int x = *(int *)(args);
	int y = *(int *)(args + sizeof(int));
	fprintf(stderr, " edge : %d\n", ((x * y) / 2) << 2);
	return ((x * y) / 2) << 2;
}

void vertexFunction(struct graph *graph, struct vertex_result* args)
{
	struct vertex_result *res = (struct vertex_result *)args;
	fprintf(stderr, "FIRING: %p, %d\n", res, *(int *) res->vertex_argv);
	*(int *) res->edge_argv += 1;
	//return res;
}



int main(){

	struct graph* graph;
	init(&graph);
	assert(graph!=NULL);

	fprintf(stderr, "TESTING GRAPH SETUP WITH MULIPLE EDGES PER VERTEX\n");

	setupVertex(graph);
	setupEdges(graph);
	runTest(graph);
	cleanup(graph);

	fprintf(stderr,"\n\nMULTI-EDGE TEST COMPLETE\n");

	return 0;
}

void init(struct graph** graph){

	*graph = graph_init(100, START_STOP, MAX_LOOPS, VERTICES | EDGES | FUNCTIONS | GLOBALS, SWITCH, CONTINUE);
	assert(*graph != NULL);


}

void setupVertex(struct graph* graph){
	assert(graph!=NULL);

	//Setting up graphs and whatnot
	//
	int i = 0;
	for (i = 0; i < MAXIMUM; i++)
	{
		int id = i;
		void (*f)(struct graph *, struct vertex_result*) = vertexFunction;
		//struct vertex_result *(*f)(void *) = vertexFunction;
		void *glbl = NULL;
		struct vertex_request *vert_req = malloc(sizeof(struct vertex_request));
		vert_req->graph = graph;
		vert_req->id = id;
		vert_req->f = f;
		vert_req->glbl = glbl;
		struct request *req = create_request(CREAT_VERTEX, vert_req, NULL);
		assert(submit_request(graph, req) == 0);
	}
	assert(process_requests(graph) == 0);

}

void setEdge(struct graph* graph, struct vertex* a, struct vertex* b){
	assert(graph!=NULL);
	assert(a!=NULL);
	assert(b!=NULL);

	struct edge_request *edge_req = malloc(sizeof(struct edge_request));
	assert(edge_req != NULL);
	edge_req->a = a;
	edge_req->b = b;
	edge_req->f = &edgeFunction;
	edge_req->glbl = NULL;
	struct request *req = create_request(CREAT_EDGE, edge_req, NULL);
	assert(submit_request(graph, req) == 0);

	fprintf(stderr, "Created edge between vertices %d and %d\n", a->id, b->id);
}



void setupEdges(struct graph* graph){
	assert(graph!=NULL);

	struct vertex* arr[MAXIMUM];
	int i = 0;
	for(i = 0; i< MAXIMUM;i++){
		arr[i] = (struct vertex*) find(graph->vertices, i);
		assert(arr[i]!=NULL);
	}
	setEdge(graph, arr[0], arr[1]);
	setEdge(graph, arr[1], arr[2]);
	setEdge(graph, arr[1], arr[3]);
	setEdge(graph, arr[0], arr[5]);
	setEdge(graph, arr[2], arr[5]);
	setEdge(graph, arr[2], arr[3]);
	setEdge(graph, arr[4], arr[5]);
	setEdge(graph, arr[3], arr[4]);
	setEdge(graph, arr[0], arr[4]);
	setEdge(graph, arr[3], arr[5]);
	setEdge(graph, arr[1], arr[5]);
	//setEdge(arr[5], arr[5]);
	assert(process_requests(graph)==0);

}


void runTest(struct graph* graph){
	assert(graph!=NULL);

	int ids[1] = {0};
	assert(start_set(graph, ids, 1)==0);

	struct vertex_result** v_args = malloc(sizeof(struct vertex_result));
	assert(v_args!=NULL);
	
	int i = 0;	
	
	int edge_args[2] = {i + 1, i + 4};
	void *edge = malloc(sizeof(int) * 2);
	memcpy(edge, &edge_args[0], sizeof(int));
	memcpy(edge + sizeof(int), &edge_args[1], sizeof(int));
	void *vertex = malloc(sizeof(int));
	*(int *)vertex = 10;
	
	struct vertex_result *v = malloc(sizeof(struct vertex_result));
	v->edge_argv = edge;
	v->edge_size = sizeof(int)*2;
	v->vertex_argv = vertex;
	v->vertex_size = sizeof(int);
	v_args[i] = v;
	
	
	assert(run(graph, v_args) == 0);
	
	fprintf(stderr,"\n\nCOMPLETED RUN\n");

}

void cleanup(struct graph* graph){
	assert(graph!=NULL);
	assert(graph != NULL);
	destroy_graph(graph);
	graph = NULL;






}
