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

int edgeFunction(void* args){

	int x = *(int*)(args);
	int y = *(int*)(args+4);
	return x*y;

}

struct vertex_result* vertexFunction(void* args){
	struct vertex_result* res = malloc(sizeof(struct vertex_result));
	if(res==NULL) return NULL;

	int counter = 0;

	int vertex_args = *(int*)(args+counter); counter+=sizeof(int);
	memcpy(res->vertex_argv, args+counter, sizeof(void*)*vertex_args); counter+=(sizeof(void*)*vertex_args);
	int edge_args = *(int*)(args+counter); counter+=sizeof(int);
	memcpy(res->edge_argv, (args+counter), sizeof(void*)*edge_args);
	return res;
	
	
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
	/*for(i =0; i< MAX_VERTICES; i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		struct vertex* v2 = (struct vertex*)find(graph->vertices, ((i+1)>=MAX_VERTICES ? 0 : i+1)); 
		assert(v!=NULL);
		assert(v2!=NULL);
		struct edge* e = (struct edge *) find(v->edge_tree, v2->id);
		if(e->glbl) {free(e->glbl); e->glbl = NULL;}
		assert(remove_edge(graph,v, v2)==0);
	}*/
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
		struct vertex_result*(*f)(void*) = vertexFunction;
		void* glbl=malloc(DEFAULT_BUFFER);
		assert(glbl!=NULL);
		void* args = malloc(sizeof(struct graph)+sizeof(struct vertext_result*(void*))+sizeof(int)+DEFAULT_BUFFER);
		int counter = 0;
		memcpy(args+counter, graph, sizeof(struct graph)); counter+=sizeof(graph);
		memcpy(args+counter, f, sizeof(struct vertex_result*(void*))); counter+=sizeof(struct vertex_result*(void*));
		memcpy(args+counter, &id, sizeof(int)); counter+=sizeof(int);
		memcpy(args+counter, glbl, DEFAULT_BUFFER);
		assert((request=CREATE_REQUEST(CREAT_VERTEX, args))!=NULL);
		free(request);
		free(glbl);
		free(args);
	}
	fprintf(stderr, "REQUEST CREATION PASSED\n");

}

void test_submit_request(struct graph* graph){
	assert(graph!=NULL);

	int i = 0;
	for(i=0; i<MAX_VERTICES; i++){
		int id = i;
		struct vertex_result*(*f)(void*) = vertexFunction;
		void* glbl = NULL;
		glbl=malloc(DEFAULT_BUFFER);
		assert(glbl!=NULL);
		void* args = malloc(sizeof(struct graph)+sizeof(struct vertext_result*(void*))+sizeof(int)+DEFAULT_BUFFER+3);
		int counter = 0;
		memcpy(args+counter, graph, sizeof(struct graph)); 
		counter+=sizeof(graph)+1;
		memcpy(args+counter, f, sizeof(struct vertex_result*(void*))); 
		counter+=sizeof(struct vertex_result*(void*))+1;
		memcpy(args+counter, &id, sizeof(int)); 
		counter+=sizeof(int)+1;
		memcpy(args+counter, glbl, DEFAULT_BUFFER);


		//struct request* req = CREATE_REQUEST(CREAT_VERTEX, args);
		struct request* req = malloc(sizeof(struct request));
		assert(req!=NULL);
		req->request=CREAT_VERTEX;
		req->args = args;
		req->f=NULL;
		assert(req!=NULL);
		assert(submit_request(graph, req)==0);
		//free(args);
		//free(req);
		free(glbl);
	}


	/*for(i=0; i<MAX_EDGES; i++){

		int (*f)(void*) = &edgeFunction;
		void* glbl = malloc(DEFAULT_BUFFER);
		struct vertex* a = (struct vertex*)find(graph->vertices, (i));
		assert(a!=NULL);
		struct vertex* b;
		if(i+1==MAX_VERTICES){
				b = (struct vertex*) find(graph->vertices, (0));
		}else{
				b = (struct vertex*)find(graph->vertices, (i+1));
		}
		assert(b!=NULL);
		//struct edge* edge;
		
		int counter = 0;
		void* args = malloc(sizeof(struct vertex)*2 + sizeof(int(void*)) + DEFAULT_BUFFER);
		//memcpy(args+counter, graph, sizeof(struct graph)); counter+=sizeof(graph);
		memcpy(args+counter, a, sizeof(struct vertex)); counter+=sizeof(struct vertex);
		memcpy(args+counter, b, sizeof(struct vertex)); counter+=sizeof(struct vertex);

		memcpy(args+counter, f, sizeof(struct vertex_result*(void*))); counter+=sizeof(struct vertex_result*(void*));
		memcpy(args+counter, glbl, DEFAULT_BUFFER);
		assert(submit_request(graph, CREATE_REQUEST(CREAT_EDGE, args))==0);
	}*/

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


