#include"../include/topologic.h"


void test_graph_insert_vertex(struct graph*);
//void test_run_single_enum(struct graph*);
//void test_run_none_enum(struct graph*);
//void test_run_switch_enum(struct graph*);
void test_graph_add_edge(struct graph*);
void test_graph_add_bi_edge(struct graph*);
void test_graph_modify_vertex(struct graph*);
void test_graph_modify_edge(struct graph*);
void test_graph_modify_bi_edge(struct graph*);
void test_graph_remove_vertex(struct graph*);
void test_graph_remove_edge(struct graph*);
void test_graph_remove_bi_edge(struct graph*);

#define TEST_SIZE 100
#define TEST_ARGC 3

struct vertex_result* testFunction(void* argv){
	struct vertex_result* res = malloc(sizeof(struct vertex_result));
	if(!res) return NULL;
	return res;
}

int testFuncEdge(void* args){
	return 0;
}



int main(){
	struct graph* graph = GRAPH_INIT(); //Start with basic setup; 
	assert(graph!=NULL);

	fprintf(stderr, "BEGINNING GRAPH OPERATIONS: VERTICES\n");
		
	//Adding
	test_graph_insert_vertex(graph);
	test_graph_add_edge(graph);
	

	//Modification


	//Cleanup
	test_graph_remove_vertex(graph);

	//End it all;
	destroy_graph(graph);
	//free(graph);
	graph=NULL;
	assert(graph==NULL);

	return 0;
}

void test_graph_insert_vertex(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
		//Dummy values
		int id = i;
		int glblc=TEST_ARGC;
		struct vertex_result*(*f)(void*) = &testFunction;
		void* glbl = malloc(32); //32 bytes of just random stuff for now
		assert(create_vertex(graph, f, id, glblc, glbl)!=NULL);
	}
	fprintf(stderr, "VERTEX INSERTION INTO GRAPH PASSED\n");
}

void test_graph_add_edge(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
		int glblc=TEST_ARGC;
		int (*f)(void*) = &testFuncEdge;
		void* glbl=malloc(32);

		struct vertex* a = find(graph->vertices, (i));
		assert(a!=NULL);
		struct vertex* b;
		if(i+1==TEST_SIZE){
				b = find(graph->vertices, (0));
		}else{
				b = find(graph->vertices, (i+1));
		}
		assert(b!=NULL);

		assert(create_edge(a, b, f, glblc, glbl)!=NULL);
		

	}
	
	fprintf(stderr,"EDGE INSERTION INTO GRAPH PASSED\n");
}

void test_graph_remove_vertex(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){	
		struct vertex* v = (struct vertex* )find(graph->vertices, i);
		assert(v!=NULL);
		assert(remove_vertex(graph, v)==0);
	}
	fprintf(stderr, "VERTEX REMOVAL FROM GRAPH PASSED\n");
}


		


