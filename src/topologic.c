// SPDX-License-Identifier: MIT WITH bison-exception

#include "../include/topologic.h"

int start_set(struct graph *graph, int *id, int num_vertices)
{
    if (!graph)
        return -1;
    if (!id)
        return -1;
    if (num_vertices < 0 || (graph->context == SINGLE && num_vertices > 1))
        return -1;

    int i = 0;
    for (; i < num_vertices; i++)
    {
        struct vertex *v = find(graph->vertices, id[i]);
        if (!v || push(graph->start, v) < 0)
        {
            /** Handle errors **/
            /**Given vertx failed, so at this point, free the vertices and leave**/
            while (pop(graph->start) != NULL)
            {
            }
            //destroy_graph(graph);
            return -1;
        }
    }
    return 0;
}

void run_single(struct graph *graph, void **init_vertex_args) {
    if (!graph || graph->context != SINGLE || graph->start->length > 1 || graph->start->length == 0) {
        return;
    }
    int successor = 0;
    struct stack *edges = init_stack();
    if (!edges) return;
    struct vertex *vertex = (struct vertex *) pop(graph->start);
    if (!vertex) return;
    struct vertex_result *res = NULL;
    void *edge_argv = NULL;
    struct edge *edge = NULL;
    
    void *args = init_vertex_args[0];
    preorder(vertex->edge_tree, edges);

    while (graph->state != TERMINATE) {
        res = (struct vertex_result *) (vertex->f)(args);
        if (res)
        {
            edge_argv = res->edge_argv;
            args = res->vertex_argv;
        }
        while ((edge = (struct edge *)pop(edges)) != NULL) {
            if (successor == 0 && (int)(edge->f)(edge_argv) >= 0) {
                vertex = edge->b;
                successor = 1;
            }
        }
        free(edge_argv);
        edge_argv = NULL;
        free(res);
        res = NULL;

        process_requests(graph);
        print(graph);
        ++(graph->state_count);
        if (successor == 0) 
            graph->state = TERMINATE;
        else successor = 0;
        if (graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes) {
            graph->state = TERMINATE;
            break;
        }
    }

    if (edge_argv) {
        free(edge_argv);
        edge_argv = NULL;
    }
    if (args) {
        free(args);
        args = NULL;
    }
    if (res) {
        free(res);
        res = NULL;
    }
    destroy_stack(edges);
    edges = NULL;
}

void run(struct graph *graph, void **init_vertex_args)
{
    if (!graph->start || graph->state == TERMINATE)
    {
        //destroy_graph(graph);
        return;
    }
    if (graph->context == SINGLE) {
        run_single(graph, init_vertex_args);
        return;
    }

    int success = 0, v_index = 0;
    struct vertex *v = NULL;
    while ((v = (struct vertex *)pop(graph->start)))
    {
        if (!success)
            success = 1;
        /** TODO: Handle pthread options **/
        if (graph->context == NONE || graph->context == SWITCH)
        {
            //TODO Set up arguments in void* buffer
            struct fireable *argv = malloc(sizeof(struct fireable));
            if (!argv)
            {
                success = 0;
                break;
            }
            argv->graph = graph;
            argv->args = init_vertex_args[v_index];
            argv->color = RED;
            argv->vertex = v;
            argv->iloop = 1;

            pthread_create(&graph->thread, NULL, fire_pthread, argv);
            ++v_index;
            free(argv);
            argv = NULL;
        }
    }

    if (!success)
    {
        /** TODO: HANDLE ERRORS **/
        /**This should be enough... right? **/
        fprintf(stderr, "Failure in run: success = %d\n", success);
        int i = 0;
        for (i = 0; i < v_index; i++)
        {
            free(init_vertex_args[i]);
        }
        pthread_exit(NULL);

        //destroy_graph(graph);
        return;
    }

    print(graph);

    pthread_cond_signal(&graph->red_cond);
    while (graph->state != TERMINATE)
    {
        if (graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes) {
            graph->state = TERMINATE;
            break;
        }
        switch (graph->state)
        {
        case RED:
            if (graph->red_vertex_count == 0)
            {
                /** TODO: REAP RED **/
                process_requests(graph);
                graph->state = PRINT;
                graph->previous_color = RED;
                pthread_cond_signal(&graph->print_cond);
                graph->print_flag = 1;
            }
            break;
        case BLACK:
            if (graph->black_vertex_count == 0)
            {
                /** TODO: REAP BLACK **/
                process_requests(graph);
                graph->state = PRINT;
                graph->previous_color = BLACK;
                pthread_cond_signal(&graph->print_cond);
                graph->print_flag = 1;
            }
            break;
        case PRINT:
            if (graph->print_flag == 0)
            {
                print(graph);
                graph->state_count++;
                if (graph->previous_color == RED)
                {
                    if (graph->black_vertex_count == 0)
                        graph->state = TERMINATE;
                    else
                        pthread_cond_signal(&graph->black_cond);
                }
                else
                {
                    if (graph->red_vertex_count == 0)
                        graph->state = TERMINATE;
                    else
                        pthread_cond_signal(&graph->red_cond);
                }
            }
            break;
        default:
            pthread_exit(NULL);
            return;
        }
    }
}

int fire(struct graph *graph, struct vertex *vertex, void *args, enum STATES color, int iloop)
{
    if (!graph || !vertex)
        return -1;
    if (graph->max_loop != -1 && iloop >= graph->max_loop)
        return -1;
    enum STATES flip_color = BLACK;
    pthread_mutex_lock(&vertex->lock);

    vertex->is_active = 1;
    if (color == RED)
    {
        pthread_cond_wait(&graph->red_cond, &vertex->lock);
        pthread_mutex_lock(&graph->lock);
        graph->red_vertex_count++;
        pthread_mutex_unlock(&graph->lock);
    }
    else if (color == BLACK)
    {
        pthread_cond_wait(&graph->black_cond, &vertex->lock);
        flip_color = RED;
        pthread_mutex_lock(&graph->lock);
        graph->black_vertex_count++;
        pthread_mutex_unlock(&graph->lock);
    }
    else
    {
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    if (graph->state == TERMINATE)
    {
        pthread_mutex_lock(&graph->lock);
        if (color == RED)
            graph->red_vertex_count--;
        else
            graph->black_vertex_count--;
        pthread_mutex_unlock(&graph->lock);
        pthread_mutex_unlock(&vertex->lock);
        return -1;
    }

    void *edge_argv = NULL, *vertex_argv = NULL;
    struct vertex_result *v_res = (vertex->f)(args);
    if (v_res)
    {
        edge_argv = v_res->edge_argv;
        vertex_argv = v_res->vertex_argv;
    }

    if (args)
    {
        free(args);
        args = NULL;
    }

    struct vertex *next_vertex = NULL;
    struct stack *edges = init_stack();
    preorder(vertex->edge_tree, edges);
    struct edge *edge = NULL;
    while ((edge = (struct edge *)pop(edges)) != NULL)
    {
        if (edge->edge_type == BI_EDGE)
        {
            pthread_mutex_lock(&edge->bi_edge_lock);
        }
        if ((int)(edge->f)(edge_argv) >= 0)
        {
            if (edge->edge_type == BI_EDGE)
            {
                pthread_mutex_unlock(&edge->bi_edge_lock);
            }
            if (graph->context == SWITCH)
            {
                int iloop_b = 1;
                if (edge->b == vertex)
                    iloop_b = iloop + 1;
                if (switch_vertex(graph, edge->b, vertex_argv, flip_color, iloop_b) < 0)
                {
                    pthread_mutex_lock(&graph->lock);
                    if (color == RED)
                        graph->red_vertex_count--;
                    else
                        graph->black_vertex_count--;
                    pthread_mutex_unlock(&graph->lock);
                    pthread_mutex_unlock(&vertex->lock);
                    return -1;
                }
            }
            else if (graph->context == NONE) {
                next_vertex = edge->b;
                break;
            }
        }
        else if (edge->edge_type == BI_EDGE)
        {
            pthread_mutex_unlock(&edge->bi_edge_lock);  
        }
    }
    destroy_stack(edges);
    edges = NULL;

    if (v_res->edge_argv)
    {
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
    vertex->is_active = 0;
    pthread_mutex_unlock(&vertex->lock);
    
    int iloop_b = 1;
    if (next_vertex == vertex)
        iloop_b = iloop + 1;
    if (graph->context == NONE && next_vertex != NULL)
        return fire(graph, next_vertex, vertex_argv, flip_color, iloop_b);
    return 0;
}

void *fire_pthread(void *vargp)
{
    if (!vargp)
        return (void *)(intptr_t)-1;
    struct fireable *fireable = (struct fireable *)vargp;

    struct graph *graph = fireable->graph;
    struct vertex *v = fireable->vertex;
    void *args = fireable->args;
    enum STATES color = fireable->color;
    int iloop = fireable->iloop;

    int ret_val = fire(graph, v, args, color, iloop);
    pthread_exit((void *)(intptr_t)ret_val);
    return (void *)(intptr_t)ret_val;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, void *args, enum STATES color, int iloop)
{
    //HANDLE STUFF LIKE THREADS HERE
    //Check if graph context = single, none, or switch?)

    struct fireable *argv = malloc(sizeof(struct fireable));
    if (!argv)
        return -1;
    argv->graph = graph;
    argv->vertex = vertex;
    argv->args = args;
    argv->color = color;
    argv->iloop = iloop;
    pthread_create(&graph->thread, NULL, fire_pthread, argv);
    free(argv);

    return 0;
}
