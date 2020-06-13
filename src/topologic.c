// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

int start_set(struct graph *graph, int id[], int num_vertices)
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

int run_single(struct graph *graph, struct vertex_result **init_vertex_args)
{
    if (!graph || graph->context != SINGLE || graph->start->length > 1 || graph->start->length == 0)
    {
        return -1;
    }
    int successor = 0;

    struct vertex *vertex = (struct vertex *)pop(graph->start);
    if (!vertex)
        return -1;
    struct edge *edge = NULL;

    struct vertex_result *args = init_vertex_args[0];
    free(init_vertex_args);
    init_vertex_args = NULL;
    struct stack *edges = init_stack();
    if (!edges)
        return -1;

    int ret = 0;
    int iloop = 0;
    while (graph->state != TERMINATE)
    {
        vertex->is_active = 1;
        print_graph(graph);
        preorder(vertex->edge_tree, edges);
        pthread_mutex_lock(&graph->lock);
        while (graph->pause)
        {
            pthread_cond_wait(&graph->pause_cond, &graph->lock);
        }
        pthread_mutex_unlock(&graph->lock);
        (vertex->f)(args);
        while ((edge = (struct edge *)pop(edges)) != NULL)
        {
            if (successor == 0 && (int)(edge->f)(args->edge_argv) >= 0)
            {
                vertex->is_active = 0;
                if (vertex == edge->b)
                    ++iloop;
                else
                    iloop = 0;
                vertex = edge->b;
                successor = 1;
            }
        }

        if (process_requests(graph) < 0)
        {
            graph->state = TERMINATE;
            ret = -1;
            break;
        }
        ++(graph->state_count);
        if (successor == 0)
            graph->state = TERMINATE;
        else
            successor = 0;
        if ((graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes) ||
            (graph->max_loop != -1 && iloop >= graph->max_loop))
        {
            graph->state = TERMINATE;
            break;
        }
    }

    if (args->edge_argv)
    {
        free(args->edge_argv);
        args->edge_argv = NULL;
    }
    if (args->vertex_argv)
    {
        free(args->vertex_argv);
        args->vertex_argv = NULL;
    }
    if (args)
    {
        free(args);
        args = NULL;
    }
    destroy_stack(edges);
    edges = NULL;
    return ret;
}

int run(struct graph *graph, struct vertex_result **init_vertex_args)
{
    if (!graph->start || graph->state == TERMINATE)
    {
        //destroy_graph(graph);
        return -1;
    }
    if (graph->context == SINGLE)
    {
        return run_single(graph, init_vertex_args);
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
								printf("fireable struct is null\n");
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
            init_vertex_args[i] = NULL;
        }
        pthread_exit(NULL);

        //destroy_graph(graph);
        return -1;
    }
    free(init_vertex_args);
    init_vertex_args = NULL;

    print_graph(graph);

    pthread_cond_signal(&graph->red_cond);
    while (graph->state != TERMINATE)
    {
        pthread_mutex_lock(&graph->lock);
        while (graph->pause)
        {
            pthread_cond_wait(&graph->pause_cond, &graph->lock);
        }
        if (graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes)
        {
            graph->state = TERMINATE;
            break;
        }
        switch (graph->state)
        {
        case RED:
            if (graph->red_vertex_count == 0)
            {
                /** TODO: REAP RED **/
                graph->state = PRINT;
                graph->previous_color = RED;
                //pthread_cond_signal(&graph->print_cond);
                graph->print_flag = 1;
            }
            break;
        case BLACK:
            if (graph->black_vertex_count == 0)
            {
                /** TODO: REAP BLACK **/
                graph->state = PRINT;
                graph->previous_color = BLACK;
                //pthread_cond_signal(&graph->print_cond);
                graph->print_flag = 1;
            }
            break;
        case PRINT:
            if (graph->print_flag == 0)
            {
                pthread_mutex_unlock(&graph->lock);
                if (process_requests(graph) < 0)
                {
                    graph->state = TERMINATE;
                    return -1;
                }
                print_graph(graph);
                pthread_mutex_lock(&graph->lock);
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
						graph->print_flag=0;
            break;
        default:
            pthread_mutex_unlock(&graph->lock);
            pthread_exit(NULL);
            return -1;
        }
        pthread_mutex_unlock(&graph->lock);
    }
    return 0;
}

int fire(struct graph *graph, struct vertex *vertex, struct vertex_result *args, enum STATES color, int iloop)
{
    if (!graph || !vertex || (graph->max_loop != -1 && iloop >= graph->max_loop))
    {
        if (args->edge_argv)
        {
            free(args->edge_argv);
            args->edge_argv = NULL;
        }
        if (args->vertex_argv)
        {
            free(args->vertex_argv);
            args->vertex_argv = NULL;
        }
        if (args)
        {
            free(args);
            args = NULL;
        }
        return -1;
    }
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
        if (args->edge_argv)
        {
            free(args->edge_argv);
            args->edge_argv = NULL;
        }
        if (args->vertex_argv)
        {
            free(args->vertex_argv);
            args->vertex_argv = NULL;
        }
        if (args)
        {
            free(args);
            args = NULL;
        }
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
        if (args->edge_argv)
        {
            free(args->edge_argv);
            args->edge_argv = NULL;
        }
        if (args->vertex_argv)
        {
            free(args->vertex_argv);
            args->vertex_argv = NULL;
        }
        if (args)
        {
            free(args);
            args = NULL;
        }
        return -1;
    }

    (vertex->f)(args);

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
        if ((int)(edge->f)(args->edge_argv) >= 0)
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
                if (switch_vertex(graph, edge->b, args, flip_color, iloop_b) < 0)
                {
                    pthread_mutex_lock(&graph->lock);
                    if (color == RED)
                        graph->red_vertex_count--;
                    else
                        graph->black_vertex_count--;
                    pthread_mutex_unlock(&graph->lock);
                    pthread_mutex_unlock(&vertex->lock);
                    if (args->edge_argv)
                    {
                        free(args->edge_argv);
                        args->edge_argv = NULL;
                    }
                    if (args->vertex_argv)
                    {
                        free(args->vertex_argv);
                        args->vertex_argv = NULL;
                    }
                    if (args)
                    {
                        free(args);
                        args = NULL;
                    }
                    return -1;
                }
            }
            else if (graph->context == NONE)
            {
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
        return fire(graph, next_vertex, args, flip_color, iloop_b);
    else
    {
        if (args->edge_argv)
        {
            free(args->edge_argv);
            args->edge_argv = NULL;
        }
        if (args->vertex_argv)
        {
            free(args->vertex_argv);
            args->vertex_argv = NULL;
        }
        if (args)
        {
            free(args);
            args = NULL;
        }
    }
		pthread_join(graph->thread, NULL);
    return 0;
}

void *fire_pthread(void *vargp)
{
    if (!vargp)
        return (void *)(intptr_t)-1;
    struct fireable *fireable = (struct fireable *)vargp;

    struct graph *graph = fireable->graph;
    struct vertex *v = fireable->vertex;
    struct vertex_result *args = fireable->args;
    enum STATES color = fireable->color;
    int iloop = fireable->iloop;

    int ret_val = fire(graph, v, args, color, iloop);
    pthread_exit((void *)(intptr_t)ret_val);
    return (void *)(intptr_t)ret_val;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, struct vertex_result *args, enum STATES color, int iloop)
{
    //HANDLE STUFF LIKE THREADS HERE
    //Check if graph context = single, none, or switch?)

    struct fireable *argv = malloc(sizeof(struct fireable));
    if (!argv)
        return -1;
    argv->args = malloc(sizeof(struct vertex_result));
    if (!argv->args)
    {
        free(argv);
        return -1;
    }
    argv->args->vertex_argv = malloc(sizeof(args->vertex_size));
    if (!argv->args->vertex_argv)
    {
        free(argv->args);
        free(argv);
        return -1;
    }
    argv->args->edge_argv = malloc(sizeof(args->edge_size));
    if (!argv->args->vertex_argv)
    {
        free(argv->args->vertex_argv);
        free(argv->args);
        free(argv);
        return -1;
    }
    memcpy(argv->args->vertex_argv, args->vertex_argv, args->vertex_size);
    memcpy(argv->args->edge_argv, args->edge_argv, args->edge_size);
    argv->args->vertex_size = args->vertex_size;
    argv->args->edge_size = args->edge_size;
    argv->graph = graph;
    argv->vertex = vertex;
    argv->color = color;
    argv->iloop = iloop;
    pthread_create(&graph->thread, NULL, fire_pthread, argv);
    free(argv);

    return 0;
}

int pause_graph(struct graph *graph)
{
    if (!graph)
        return -1;
    if (graph->pause == 1)
        return -1;

    pthread_mutex_lock(&graph->lock);
    graph->pause = 0;
    pthread_cond_signal(&graph->pause_cond);
    pthread_mutex_unlock(&graph->lock);
    return 0;
}

int resume_graph(struct graph *graph)
{
    if (!graph)
        return -1;
    if (graph->pause == 0)
        return -1;

    pthread_mutex_lock(&graph->lock);
    graph->pause = 1;
    pthread_mutex_unlock(&graph->lock);
    return 0;
}
