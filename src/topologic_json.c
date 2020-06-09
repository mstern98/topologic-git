#include "../include/topologic.h"

void print_edges(struct graph *graph, struct AVLTree *edges, const char *indent) {
    struct stack *stack = init_stack();
    struct edge *edge = NULL;
    preorder(edges, stack);
    printf("%sEdges: {", indent);
    while ((edge = (struct edge *) pop(stack)) != NULL) {
        printf("%s\tid: %d\n", indent, edge->id);
        printf("%s\tfrom: %p\n", indent, edge->a);
        printf("%s\tto: %p\n", indent, edge->b);
        printf("%s\tedge_type: %d\n", indent, edge->edge_type);
        printf("%s\tbi_edge: %p\n", indent, edge->bi_edge);
        if ((graph->lvl_verbose & FUNCTIONS) == FUNCTIONS) 
            printf("%s\tf: %p\n", indent, edge->f);
        if ((graph->lvl_verbose & GLOBALS) == GLOBALS) {
                        
            printf("%s\tglbl: %p\n", indent, edge->glbl);
        }
    }
    destroy_stack(stack);

}

void print_state(struct graph *graph)
{
    /*Called by print and does a pre-order traversal of all the data in each vertex*/
    int vertex_id = 0;
		void *glbl = NULL;
    //int edge_sharedc;
    union shared_edge *edge_shared = NULL;
    struct vertex_result *(*f)(void *) = NULL;
    struct vertex *v = NULL;
    struct stack *stack = init_stack();

    inorder(graph->vertices, stack);
    while ((v = (struct vertex *) pop(stack)) != NULL) {
        vertex_id = v->id;
        f = v->f;
        edge_shared = v->shared;
        glbl = v->glbl;

        //TODO: Setup on Verbose
        if ((graph->lvl_verbose & VERTICES) == VERTICES) {
            printf("\tVertex: {\n");
            printf("\t\tid: %d\n", vertex_id);
            printf("\t\tactive: %d\n", v->is_active);
            if ((graph->lvl_verbose & FUNCTIONS) == FUNCTIONS) {
                printf("\t\tf: %p\n", f);
            }
            if ((graph->lvl_verbose & GLOBALS) == GLOBALS) {
                
                printf("\t\tglbl: %p\n", glbl);
                printf("\t\tglbl: [");
                //printf("\t\tedge_sharedc: %d\n", edge_sharedc);
                printf("\t\tedge_shared: %p\n", edge_shared);
            }
            if ((graph->lvl_verbose & EDGES) == EDGES) {
                print_edges(graph, v->edge_tree, "\t\t\t");
            }
            printf("\t},\n");
        } else if ((graph->lvl_verbose & EDGES) == EDGES) {
            print_edges(graph, v->edge_tree, "\t\t");
        }
    }
    destroy_stack(stack);
}

void print(struct graph *graph)
{
    int out_fd = fileno(stdout);
    if (!graph)
        return;

    pthread_mutex_lock(&graph->lock);
    if (graph->lvl_verbose == NO_VERB)
    {
        pthread_mutex_unlock(&graph->lock);
        return;
    }

    int dirfd = open("./", O_DIRECTORY | O_RDONLY);
    if (dirfd == -1) return;
    char buffer[256];
    sprintf(buffer, "state_%d.json", graph->state_count);
    int fd = openat(dirfd, buffer, O_CREAT | O_WRONLY, S_IRWXU);
    if (fd == -1) {
        close(dirfd);
        return;
    }
    dup2(out_fd, fd);

    /**TODO: Print enums**/
    printf("{\n");
    printf(" graph: {\n");
    printf("\tstate: %d\n\tmax state repeats: %d\n\ttimestamps: %d\n\tverbosity: %d\n\tnodes: %d\n", graph->state_count,
    graph->max_state_changes, 
    graph->snapshot_timestamp,
    graph->lvl_verbose,
    graph->vertices->size);
    print_state(graph);
    printf(" }\n");
    printf("}");

    close(fd);
    close(dirfd);
    dup2(fd, out_fd);
    pthread_mutex_unlock(&graph->lock);
}
