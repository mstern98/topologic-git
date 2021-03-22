// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"
#include "../include/test.h"

void init(struct graph**);


void test_create_request(struct graph*);
void test_submit_request(struct graph*);
void test_process_requests(struct graph*);

void test_destroy_request(struct graph*);
void cleanup(struct graph*);


#define MAX_VERTICES 100
#define MAX_EDGES MAX_VERTICES
#define DEFAULT_BUFFER 32

int edgeFunction(int id, void* args, void *glbl, const void *const edge_vars_a, const void *const edge_vars_b){

	int x = *(int*)(args);
	int y = *(int*)(args+4);
	return x*y;

}

void vertexFunction(int id, struct graph *graph, struct vertex_result* args, void *glbl, void *edge_vars){
}

int main(){

	//Setup graph
	struct graph* graph;
	init(&graph);
	assert(graph!=NULL);
	
	fprintf(stderr, "PREPARING TO TEST REQUEST MECHANISM\n");
	test_create_request(graph);
	test_submit_request(graph);
	test_process_requests(graph);
	test_destroy_request(graph);

	cleanup(graph);
	//destroy_graph(graph);
	graph=NULL;
	fprintf(stderr, "REQUEST TESTS PASSED\n");

	return 0;
}

void cleanup(struct graph* graph){
	assert(graph!=NULL);
	int i = 0;

	for(i = 0; i<MAX_VERTICES; i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		if(v==NULL) continue;
		if(v->glbl){free(v->glbl); v->glbl=NULL;}
		remove_vertex(graph,v);
	}
	destroy_graph(graph);
	graph=NULL;
}	

void init(struct graph** graph){
	*graph = GRAPH_INIT();
	assert(*graph!=NULL);
}

void test_create_request(struct graph* graph){
	assert(graph!=NULL);

	int i = 0;
	for(i=0; i<MAX_VERTICES; i++){
		struct request* request;
		int id = i;
		void (*f)(int, struct graph *, struct vertex_result*, void *, void *) = vertexFunction;
		void* glbl = NULL;
		struct vertex_request *v_req = malloc(sizeof(struct vertex_request));

		v_req->f = f;
		v_req->glbl = glbl;
		v_req->graph = graph;
		v_req->id = id;
		assert((request=CREATE_REQUEST(CREAT_VERTEX, v_req))!=NULL);
		free(request);
		free(v_req);
	}
	fprintf(stderr, "REQUEST CREATION PASSED\n");

}

void test_submit_request(struct graph* graph){
	assert(graph!=NULL);

	int i = 0;
	for(i=0; i<MAX_VERTICES; i++){
		int id = i;
		void (*f)(int, struct graph *, struct vertex_result*, void *, void *) = vertexFunction;
		void* glbl = NULL;
		
		struct vertex_request *vert_req = malloc(sizeof(struct vertex_request));
		vert_req->graph = graph;
		vert_req->id=id;
		vert_req->f = f;
		vert_req->glbl = glbl;

		struct request *req = create_request(CREAT_VERTEX, vert_req, NULL);
		assert(submit_request(graph, req)==0);
		//free(glbl);
	}
	fprintf(stderr, "REQUEST SUBMISSION PASSED\n");
}

void test_process_requests(struct graph* graph){
	assert(graph!=NULL);

	assert(process_requests(graph)==0);
	fprintf(stderr, "REQUEST PROCESSING HAS PASSED\n");
}

void test_destroy_request(struct graph* graph){
	assert(graph!=NULL);
	struct request* request;

	while((request=(struct request*) pop(graph->modify))!=NULL){
		destroy_request(request);
	}
	while((request=(struct request*)pop(graph->remove_edges))!=NULL){
		destroy_request(request);
	}
	while((request=(struct request*)pop(graph->remove_vertices))!=NULL){
		destroy_request(request);
	}
	//assert(graph!=NULL);
}


