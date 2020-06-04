#include "../include/topologic.h"

struct graph *graph_init(unsigned int max_state_changes, unsigned int snapshot_timestamp, enum VERBOSITY lvl_verbose, enum CONTEXT context) {
    struct graph *graph = malloc(sizeof(struct graph));
    if (!graph) return NULL;
    graph->max_state_changes = max_state_changes;
    graph->snapshot_timestamp = snapshot_timestamp;
    graph->lvl_verbose = lvl_verbose;
    graph->context = context;

    if(pthread_mutex_init(&graph->lock, NULL) < 0) {
        free(graph);
        return NULL;
    }

    if(pthread_cond_init(&graph->print_cond, NULL) < 0) {
        pthread_mutex_destroy(&graph->lock);
        free(graph);
        return NULL;
    }

    if(pthread_cond_init(&graph->red_cond, NULL) < 0) {        
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        free(graph);
        return NULL;
    }

    if(pthread_cond_init(&graph->black_cond, NULL) < 0) {        
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        free(graph);
        return NULL;
    }

    graph->vertices = init_avl();
    if(!graph->vertices) {
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        pthread_cond_destroy(&graph->black_cond);
        free(graph);
        return NULL;
    }

    graph->modify = init_stack();
    if (!graph->modify) {
        destroy_avl(graph->vertices);
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        pthread_cond_destroy(&graph->black_cond);
        free(graph);
        return NULL;
    }
    graph->remove_edges = init_stack();
    if (!graph->remove_edges) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        pthread_cond_destroy(&graph->black_cond);
        free(graph);
        return NULL;
    }
    graph->remove_vertices = init_stack();
    if (!graph->remove_vertices) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        destroy_stack(graph->remove_edges);
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        pthread_cond_destroy(&graph->black_cond);
        free(graph);
        return NULL;
    }
    graph->start = init_stack();
    if (!graph->start) {
        destroy_avl(graph->vertices);
        destroy_stack(graph->modify);
        destroy_stack(graph->remove_edges);
        destroy_stack(graph->remove_vertices);
        pthread_mutex_destroy(&graph->lock);
        pthread_cond_destroy(&graph->print_cond);
        pthread_cond_destroy(&graph->red_cond);
        pthread_cond_destroy(&graph->black_cond);
        free(graph);
        return NULL;
    }

    graph->previous_color = RED;
    graph->state = RED;
    graph->red_vertex_count = 0;
    graph->black_vertex_count = 0;
    graph->print_flag = 0;
    
    return graph;
}

int start_set(struct graph *graph, struct vertex **vertices, int num_vertices) {
    if(!graph) return -1;
    if(!vertices) return -1;
    if(num_vertices<0) return -1;

    int i = 0;
    for(;i<num_vertices; i++){
        struct vertex* v = vertices[i];
        if(!v) return -1;
        if (push(graph->start, v) < 0) return -1;
        fire(graph, v, v->argc, vertex_args[i], RED);
    }
    return 0;
}

void process_requests(struct graph *graph) {
    if (!graph) return;
    
    pthread_mutex_lock(&graph->lock);

    struct request *req = NULL;
    while ((req = (struct request *) pop(graph->modify)) != NULL)
        (req->f)(req->args);
    while ((req = (struct request *) pop(graph->remove_edges)) != NULL)
        (req->f)(req->args);
    while ((req = (struct request *) pop(graph->remove_vertices)) != NULL)
        (req->f)(req->args);

    pthread_mutex_unlock(&graph->lock);
}

void run(struct graph *graph,  void **init_vertex_args[]) {

    if (!graph->start) return;
    int success = 0, v_index = 0;
    struct vertex *v = NULL;
    while ((v = (struct vertex *) pop(graph->start))) {
        if (!success) success = 1;
        /** TODO: Handle pthread options **/
        fire(graph, v, v->argc, init_vertex_args[v_index], RED);
        ++v_index;
    }

    if (!success) {
        /** TODO: HANDLE ERRORS **/
        return;
    }

    pthread_cond_signal(&graph->red_cond);
    while(1) {
        switch(graph->state) {
            case RED:
                if (graph->red_vertex_count == 0) {
                    /** TODO: REAP RED **/
                    process_requests(graph);
                    graph->state = PRINT;
                    graph->previous_color = RED;
                    pthread_cond_signal(&graph->print_cond);
                    graph->print_flag = 1;
                }
            break;
            case BLACK:
                if (graph->black_vertex_count == 0) {
                    /** TODO: REAP BLACK **/
                    process_requests(graph);
                    graph->state = PRINT;
                    graph->previous_color = BLACK;
                    pthread_cond_signal(&graph->print_cond);
                    graph->print_flag = 1;
                }
            break;
            case PRINT:
                if (graph->print_flag == 0) {
                    if (graph->previous_color == RED) {
                        pthread_cond_signal(&graph->black_cond);
                    } else {
                        pthread_cond_signal(&graph->red_cond);
                    }
                }
            break;
            default:
                return;
        }
    }
}


void print_state(struct AVLNode* node){
    /*Called by print and does a pre-order traversal of all the data in each vertex*/
    int counter = 0;
    void* data;

    int vertexId=0, argc=0, glblc=0;
    void* glbl=NULL;
    int edge_sharedc;
    void *edge_shared=NULL;
    void (*f)(void *);
    struct AVLNode* left = NULL;
    struct AVLNode* right = NULL;
    if(!node){ return;}

    left = node->left;
    right=node->right;
    data = node->data;

    void* temp;

    memcpy(&vertexId, (data+counter), sizeof(int));
    counter+=sizeof(int);
    temp = data+counter;
    temp = &(temp);
    memcpy(&f, (&temp), sizeof(void*)); counter+=sizeof(void*);
    memcpy(&argc, data+counter, sizeof(int)); counter+=sizeof(int);
    memcpy(&glblc, data+counter, sizeof(int)); counter+=sizeof(int);
    temp = data+counter;
    memcpy(&glbl, &temp, sizeof(void*)*glblc); counter+=(sizeof(void*)*glblc);
    memcpy(&edge_sharedc, (data+counter), sizeof(int)); counter+=sizeof(int);
    temp = data+counter;
    memcpy(&edge_shared, &temp, sizeof(void*)*edge_sharedc); counter+=(sizeof(void*)*edge_sharedc);
    
    printf("\tNode #%d: {", vertexId);
    printf("\t\tf: %p\n", f);
    printf("\t\targc: %d\n", argc);
    printf("\t\tglblc: %d\n", glblc);
    printf("\t\tglbl: %p\n", glbl);
    printf("\t\tedge_sharedc: %d\n", edge_sharedc);
    printf("\t\tedge_shared: %p\n", edge_shared);
    printf("\t},\n");
    print_state(left);
    print_state(right);
    
}

void print(struct graph *graph) {
    if(!graph) return;

    pthread_mutex_lock(&graph->lock);
    if(graph->lvl_verbose == NO_VERB) {
        pthread_mutex_unlock(&graph->lock);
        return;
    }

    /**TODO: Print enums**/
    printf("graph: {\n");
    print_state(graph->vertices->root);
    printf("}\n");
    pthread_mutex_unlock(&graph->lock);
}

struct request *create_request(enum REQUESTS request, void **args, void (*f)(void **), int argc) {
    struct request *req = malloc(sizeof(struct request));
    if (!req) return NULL;
    req->args = malloc(sizeof(void *) * argc);
    if (!req->args) {
        free(req);
        return NULL;
    }
    memcpy(req->args, args, sizeof(void *) * argc);

    req->argc = argc;
    req->f = f;
    req->request = request;
    return req;
}

int submit_request(struct graph *graph, struct request *request) {
    if (!graph || !request) return -1;
    int retval = 0;
    pthread_mutex_lock(&graph->lock);
    switch(request->request) {
        case MODIFY:
            retval = push(graph->modify, (void *) request);
            break;
        case DESTROY_EDGE:
            retval = push(graph->remove_edges, (void *) request);
            break;
        case DESTROY_VERTEX:
            retval = push(graph->remove_vertices, (void *) request);
            break;
        default:
            retval = -1;
            break;
    }
    pthread_mutex_unlock(&graph->lock);
    return retval;
}

int fire(struct graph *graph, struct vertex *vertex, int argc, void **args, enum STATES color) {
    if (!graph || !vertex) return -1;
    enum STATES flip_color = BLACK;
    pthread_mutex_lock(&vertex->lock);
    if (color == RED) {
        pthread_cond_wait(&graph->red_cond, &vertex->lock);

        pthread_mutex_lock(&graph->lock);
        graph->red_vertex_count++;
        pthread_mutex_unlock(&graph->lock);
    } else if (color == BLACK) {
        pthread_cond_wait(&graph->black_cond, &vertex->lock);
        flip_color = RED;
    } else {
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }
    if (argc != vertex->argc) {
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    int edge_argc = 0, vertex_argc = 0;
    void **edge_argv = NULL, **vertex_argv = NULL;
    struct vertex_result *v_res = (vertex->f)(argc, args);
    if (v_res) {
        edge_argc = v_res->edge_argc;
        edge_argv = v_res->edge_argv;
        vertex_argc = v_res->vertex_argc;
        vertex_argv = v_res->vertex_argv;
    }

    if (args) {
        int i = 0;
        for (i = 0; i < argc; i++) {
            free(args[i]);
            args[i] = NULL;
        }
        free(args);
    }

    struct stack *edges = init_stack();
    preorder(vertex->edge_tree, edges);
    struct edge *edge = NULL;
    while ((edge = (struct edge *) pop(edges)) != NULL) {
        if (!edge->b) {
            void **data = malloc(sizeof(void *) * 2);
            data[0] = vertex;
            data[1] = &(edge->id);
            struct request *req = create_request(DESTROY_EDGE, data, (void *) remove_edge_id, 2);
            submit_request(graph, req);
        }
        else if ((int) (edge->f)(edge_argc, edge_argv) >= 0) {
            if (switch_vertex(graph, edge->b, vertex_argc, vertex_argv, flip_color) < 0) {
                pthread_mutex_lock(&graph->lock);
                if (color == RED)
                    graph->red_vertex_count--;
                else 
                    graph->black_vertex_count--;
                pthread_mutex_unlock(&graph->lock);

                pthread_mutex_unlock(&vertex->lock);
                return -1;
            }
            if (graph->context == SINGLE || graph->context == NONE)
                break;
        }
    }
    destroy_stack(edges);

    if (v_res->edge_argv) {
        int i = 0;
        for (i = 0; i < v_res->edge_argc; i++) {
            free(v_res->edge_argv[i]);
            v_res->edge_argv[i] = 0;
        }
        free(v_res->edge_argv);
        v_res->edge_argv = NULL;
    }
    free(v_res);
    v_res = NULL;
    

    pthread_mutex_lock(&graph->lock);
    if (color == RED)
        graph->red_vertex_count--;
    else 
        graph->black_vertex_count--;
    pthread_mutex_unlock(&graph->lock);

    pthread_mutex_unlock(&vertex->lock);
    return 0;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, int argc, void **args, enum STATES color) {
    //HANDLE STUFF LIKE THREADS HERE

    fire(graph, vertex, argc, args, color);
    return 0;
}

void destroy_graph_stack(struct stack *stack) {
    if (!stack) return;
    struct request *request = NULL;
    while ((request = (struct request *) pop(stack)) != NULL) {
        destroy_request(request);
    }
    destroy_stack(stack);
}

void destroy_graph_avl(struct graph *graph, struct AVLTree *tree) {
    if (!tree) return;

    struct stack* tree_stack = malloc(sizeof(struct stack));
    if(!tree_stack) return;
    /*struct stack *stack =*/ preorder(tree, tree_stack);
    //free(tree_stack);
    struct vertex *vertex = NULL;
    while ((vertex = (struct vertex *) pop(tree_stack)) != NULL) {
        remove_vertex(graph, vertex);
    }
    free(tree_stack);
    destroy_avl(tree);
}

int destroy_graph(struct graph *graph) {
    if (!graph) return -1;
    destroy_graph_avl(graph, graph->vertices);
    destroy_graph_stack(graph->modify);
    destroy_graph_stack(graph->remove_edges);
    destroy_graph_stack(graph->remove_vertices);

    pthread_mutex_destroy(&graph->lock);
    pthread_cond_destroy(&graph->print_cond);
    pthread_cond_destroy(&graph->red_cond);
    pthread_cond_destroy(&graph->black_cond);
    free(graph);
    return 0;
}

int destroy_request(struct request *request) {
    if (!request) return -1;
    if (request->argc == 1) {
        free(request->args);
        request->args = NULL;
    } else if (request->argc > 1) {
        int i = 0;
        for(i=0; i < request->argc; i++) {
            free(request->args[i]);
            request->args[i] = NULL;
        }
        free(request->args);
    }
    request->argc = 0;
    request->request = 0;
    return 0;
}




