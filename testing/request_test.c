#include "../include/topologic.h"
#include "../include/test.h"

void init(struct graph**);
void setup_vertices(struct graph*);
void setup_edges(struct graph*);

void test_create_request(struct graph*);
void test_submit_request(struct graph*);
void test_process_requests(struct graph*);

void test_destroy_request(struct graph*);


#define MAX_VERTICES 100
#define MAX_EDGES MAX_VERTICES

int main(){

	//Setup graph
	struct graph* graph;
	init(&graph);
	assert(graph!=NULL);
	setup_vertices(graph);
	setup_edges(graph);

	fprintf(stderr, "PREPARING TO TEST REQUEST MECHANISM\n");
	test_create_request(graph);
	test_submit_request(graph);
	test_process_requests(graph);
	test_destroy_request(graph);

	destroy_graph(graph);
	graph=NULL;
	fprintf(stderr, "REQUEST TESTS PASSED\n");


	return 0;
}

void init(struct graph** graph){
	*graph = GRAPH_INIT();
	assert(*graph!=NULL);
}

void setup_vertices(struct graph* graph){
	assert(graph!=NULL);
}

void setup_edges(struct graph* graph){
	assert(graph!=NULL);

}


void test_create_request(struct graph* graph){
	assert(graph!=NULL);
}

void test_submit_request(struct graph* graph){
	assert(graph!=NULL);
}

void test_process_requests(struct graph* graph){
	assert(graph!=NULL);
}

void test_destroy_request(struct graph* graph){
	assert(graph!=NULL);
}


