#include"../include/topologic.h"
#include"../include/test.h"

void init(struct graph**);
void setup_start_set(struct graph* graph);
void setup_non_start_set(struct graph* graph);
void test_run_single(struct graph*);
void cleanup(struct graph*);

#define MAXIMUM 100
#define DEFAULT_BUFFER 64

int edgeFunction(void* args){
	int x= *(int*)(args);
	int y= *(int*)(args+sizeof(int));
	return ((x*y)/2)<< 2;
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
	struct graph* graph;
	init(&graph);
	assert(graph!=NULL);

	fprintf(stderr, "PREPARING TO TEST CONTEXT MECHANISM: \"SINGLE\" CONTEXT\n");
	setup_start_set(graph);

	test_run_single(graph);

	cleanup(graph);
	fprintf(stderr, "GRAPH CONTEXT \"SINGLE\" PASSED\n");
	return 0;
}

void cleanup(struct graph* graph){
	assert(graph!=NULL);
	int i = 0;

	for(i = 0; i<MAXIMUM; i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		if(v==NULL) continue;
		if(v->glbl){free(v->glbl); v->glbl=NULL;}
		remove_vertex(graph,v);
	}
	destroy_graph(graph);
	graph=NULL;
}	

void init(struct graph** graph){
	*graph = graph_init(-1, START_STOP, MAX_LOOPS, VERTICES | EDGES | FUNCTIONS | GLOBALS, SINGLE, CONTINUE);
	assert(*graph!=NULL);

	//Setting up graphs and whatnot
	//
	int i = 0;
	for(i=0;i<MAXIMUM;i++){
		int id = i;
		struct vertex_result*(*f)(void*)=&vertexFunction;
		void* glbl=NULL;
		struct vertex_request* vert_req = malloc(sizeof(struct vertex_request));
		vert_req->graph = *graph;
		vert_req->id = id;
		vert_req->f=f;
		vert_req->glbl=glbl;
		struct request* req = create_request(CREAT_VERTEX, vert_req, NULL);
		assert(submit_request(*graph, req)==0);
	}
	assert(process_requests(*graph)==0);
	//Now the edges: TODO
	

}

void setup_start_set(struct graph* graph){
	assert(graph!=NULL);

	/*TODO*/
}

void setup_non_start_set(struct graph* graph){
	assert(graph!=NULL);

	/*TODO*/

}

void test_run_single(struct graph* graph){
	assert(graph!=NULL);

	/*TODO*/
}



