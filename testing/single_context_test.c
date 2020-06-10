#include"../include/topologic.h"
#include"../include/test.h"

void init(struct graph**);
void setup_vertices(struct graph*);
void setup_edges(struct graph*);

#define GLBLC_COUNT 4

struct vertex_result* vertFunc(void* argv){
	if(argv==NULL){return NULL;}
	
	struct vertex_result* res = malloc(sizeof(struct vertex_result));
	if(res==NULL){return NULL;}
	res->vertex_argv=(argv);
	res->edge_argv = (argv+4);
	return res;
	
}

int edgeFunction(void* argv){
	if(argv==NULL) return -1;
	
	int x = *(int*)(argv);
	int y = *(int*)(argv+sizeof(init));
	return x*y;
}


int main(){


	return 0;
}
