#include"../include/topologic.h"

void test_graph_init(struct graph*);
void test_graph_insert_vertex(struct graph*, struct vertex**);
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

int main(){
	struct graph* graph = GRAPH_INIT(); //Start with basic setup; 
	assert(graph!=NULL);
}
