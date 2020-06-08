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

struct vertex_result* testFunction2(void* argv){
	struct vertex_result* res = malloc(sizeof(struct vertex_result));
	assert(res!=NULL);

	res->vertex_argv = (argv);
	res->edge_argv = (argv+4);
	return res;
}


int testFuncEdge(void* args){
	return 0;
}

int testFuncEdge2(void* args){
	int i = 0;
	int j = *(int*)(args+i);
	j+=i;

	return j;
}



int main(){
	struct graph* graph = GRAPH_INIT(); //Start with basic setup; 
	assert(graph!=NULL);

	fprintf(stderr, "BEGINNING GRAPH OPERATIONS: VERTEX/EDGE INSERT, MOD, DELETE\n");
		
	//Adding
	test_graph_insert_vertex(graph);

	//Modify Vertices
	test_graph_modify_vertex(graph);


	//Edges
	test_graph_add_edge(graph);
	test_graph_modify_edge(graph);
	test_graph_remove_edge(graph);

	//BiEdges
	test_graph_add_bi_edge(graph);
	test_graph_modify_bi_edge(graph);
	test_graph_remove_bi_edge(graph);
	

	//Modification


	//Cleanup
	test_graph_remove_vertex(graph);

	//End it all;
	destroy_graph(graph);
	graph=NULL;
	assert(graph==NULL);

	fprintf(stderr,"VERTEX AND EDGE TESTING COMPLETED\n");

	return 0;
}

void test_graph_insert_vertex(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
		//Dummy values
		int id = i;
		struct vertex_result*(*f)(void*) = &testFunction;
		void* glbl = malloc(32); //32 bytes of just random stuff for now
		assert(create_vertex(graph, f, id, glbl)!=NULL);
	}
	fprintf(stderr, "VERTEX INSERTION INTO GRAPH PASSED\n");
}

void test_graph_add_edge(struct graph* graph){
	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
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
		struct edge* edge;
		assert((edge=create_edge(a, b, f, glbl))!=NULL);	
	}
	
	fprintf(stderr,"EDGE INSERTION INTO GRAPH PASSED\n");
}

void test_graph_add_bi_edge(struct graph* graph){

	int i = 0;
	for(i=0; i<TEST_SIZE; i++){
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
		struct edge** edge;
		assert((edge=create_bi_edge(a, b, f, glbl))!=NULL);	

	}

	fprintf(stderr,"BI-DIRECTIONAL EDGE INSERTION INTO GRAPH PASSED\n");
}

void test_graph_modify_vertex(struct graph* graph){
	assert(graph!=NULL);

	int i = 0;
	for(i = 0; i<TEST_SIZE; i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		assert(v!=NULL);
		assert(modify_vertex(v, &testFunction2, NULL)==0);
		assert(modify_vertex(v, &testFunction, NULL)==0);
		assert(modify_vertex(v, NULL, NULL)==0);
	}
	fprintf(stderr,"VERTEX MODIFICATION PASSED\n");

}


void test_graph_modify_edge(struct graph* graph){

	assert(graph!=NULL);
	int i = 0;
	for(i=0; i<TEST_SIZE;i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		struct vertex* v2 = (struct vertex*)find(graph->vertices, ((i+1)>=TEST_SIZE ? 0 : i+1)); 
		assert(v!=NULL);
		assert(v2!=NULL);

		assert(modify_edge(v, v2, &(testFuncEdge2), NULL)==0);
		assert(modify_edge(v, v2, &testFuncEdge, NULL)==0);
		assert(modify_edge(NULL,v2, NULL, NULL)<0);
		assert(modify_edge(v,v2,NULL,NULL)==0);
	}
	fprintf(stderr, "EDGE MODIFICATION PASSED\n");


}

void test_graph_modify_bi_edge(struct graph* graph){

	assert(graph!=NULL);
	int i = 0;
	for(i=0; i<TEST_SIZE;i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		struct vertex* v2 = (struct vertex*)find(graph->vertices, ((i+1)>=TEST_SIZE ? 0 : i+1)); 
		assert(v!=NULL);
		assert(v2!=NULL);

		assert(modify_bi_edge(v, v2, &(testFuncEdge2), NULL)==0);
		assert(modify_bi_edge(v, v2, &testFuncEdge, NULL)==0);
		assert(modify_bi_edge(NULL,v2, NULL, NULL)<=-1);
		assert(modify_bi_edge(v,v2,NULL,NULL)==0);
	}
	fprintf(stderr, "EDGE MODIFICATION PASSED\n");


}



void test_graph_remove_bi_edge(struct graph* graph){
	assert(graph!=NULL);
	int i = 0;
	for(i=0; i<TEST_SIZE;i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		struct vertex* v2 = (struct vertex*)find(graph->vertices, ((i+1)>=TEST_SIZE ? 0 : i+1)); 
		assert(v!=NULL);
		assert(v2!=NULL);
		assert(remove_bi_edge(v, v2)==0); 
	}
	fprintf(stderr, "BI-DIRECTIONAL EDGE REMOVAL FROM GRAPH PASSED\n");

}

void test_graph_remove_edge(struct graph* graph){
	assert(graph!=NULL);
	int i = 0;
	for(i=0; i<TEST_SIZE;i++){
		struct vertex* v = (struct vertex*)find(graph->vertices, i);
		struct vertex* v2 = (struct vertex*)find(graph->vertices, ((i+1)>=TEST_SIZE ? 0 : i+1)); 
		assert(v!=NULL);
		assert(v2!=NULL);
		assert(remove_edge(v, v2)==0);
		 
	}
	fprintf(stderr, "EDGE REMOVAL FROM GRAPH PASSED\n");

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


		


