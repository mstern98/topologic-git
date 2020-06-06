#include"../include/topologic.h"


void test_graph_insert_vertex(struct graph*);
void test_run_single_enum(struct graph*);
void test_run_none_enum(struct graph*);
void test_run_switch_enum(struct graph*);
void test_graph_add_edge(struct graph*, struct edge**);
void test_graph_add_bi_edge(struct graph*, struct edge**[]);
void test_graph_modify_vertex(struct graph*, struct vertex*);
void test_graph_modify_edge(struct graph*, struct edge*);
void test_graph_modify_bi_edge(struct graph*, struct edge**);
void test_graph_remove_vertex(struct graph*, struct vertex**);
void test_graph_remove_edge(struct graph*, struct edge**);
void test_graph_remove_bi_edge(struct graph*, struct edge**[]);

#define TEST_SIZE 100
#define TEST_ARGC 3

struct vertex_result* testFunction(int argc, void* argv){
	struct vertex_result* res = malloc(sizeof(struct vertex_result));
	if(!res) return NULL;
	res->vertex_argc=res->edge_argc=argc;
	res->vertex_argv=res->edge_argv = argv;
	return res;
}



int main(){
	struct graph* graph = GRAPH_INIT(); //Start with basic setup; 
	assert(graph!=NULL);

	fprintf(stderr, "BEGINNING GRAPH OPERATIONS: VERTICES\n");
	/**Setup for vertex**/
	//struct vertex** vertices =malloc(sizeof(struct vertex)*TEST_SIZE);
	test_graph_insert_vertex(graph);



}

void test_graph_insert_vertex(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
		//Dummy values
		int id = i;
		int argc=TEST_ARGC;
		int glblc=TEST_ARGC;
		struct vertex_result*(*f)(int, void*) = &testFunction;
		void* glbl = malloc(32); //32 bytes of just random stuff for now
		assert(create_vertex(graph, f, id, argc, glblc, glbl)!=NULL);
	}
	fprintf(stderr, "VERTEX INSERTION INTO GRAPH PASSED\n");
}

		


