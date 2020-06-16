// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

int start_set(struct graph *graph, int id[], int num_vertices)
{
    if (!graph)
        return -1;
    if (!id)
        return -1;
    if (num_vertices <= 0 || (graph->context == SINGLE && num_vertices > 1))
        return -1;

    while (pop(graph->start) != NULL)
    {
    }

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
        (vertex->f)(graph, args);
        while ((edge = (struct edge *)pop(edges)) != NULL)
        {
            if (successor == 0 && (edge->f)(args->edge_argv))
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
        return -1;
    if (graph->context == SINGLE)
        return run_single(graph, init_vertex_args);

    int success = 0, v_index = 0;
    struct vertex *v = NULL;
    while ((v = (struct vertex *)pop(graph->start)))
    {
        if (!success)
            success = 1;

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

        //Checking result of pthread_create
        int thread_result = 0;
        int thread_attempts = 0;
        pthread_t thread;
    create_start_threads:
        thread_result = pthread_create(&thread, NULL, fire_pthread, argv);
        ++thread_attempts;
        if (thread_result != 0)
        {
            if (errno != EAGAIN)
            {
                perror("Creating initial Threads: ");
                success = -1;
                break;
            }
            else
            {
                switch (graph->mem_option)
                {
                case CONTINUE:
                    break;
                case WAIT:
                    if (thread_attempts > MAX_ATTEMPTS)
                    {
                        success = -1;
                        fprintf(stderr, "Max Threads Attempts Hit\n");
                        break;
                    }
                    sleep(THREAD_ATTEMPT_SLEEP);
                    errno = 0;
                    goto create_start_threads;
                case ABORT:
                    success = -1;
                    fprintf(stderr, "Failed to Create Threads\n");
                    break;
                }
            }
        }
        pthread_detach(thread);
        v->is_active = 1;
        ++(graph->num_vertices);
        ++v_index;

        argv = NULL;
    }

    if (!success)
    {
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

    while (graph->state != TERMINATE)
    {
        pthread_mutex_lock(&graph->lock);
        while (graph->pause)
        {
            pthread_cond_wait(&graph->pause_cond, &graph->lock);
        }
        pthread_mutex_unlock(&graph->lock);
        if (graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes)
        {
            graph->state = TERMINATE;
            graph->red_locked = 0;
            graph->black_locked = 0;
            break;
        }
        switch (graph->state)
        {
        case RED:
            pthread_mutex_lock(&graph->lock);
            graph->red_locked = 0;
            graph->black_locked = 1;
            pthread_mutex_unlock(&graph->lock);
            pthread_cond_wait(&graph->red_fire, &graph->color_lock);
            pthread_mutex_lock(&graph->lock);
            if (graph->red_vertex_count == 0)
            {
                graph->state = PRINT;
                graph->previous_color = RED;
                graph->print_flag = 1;
                //fprintf(stderr, "WAS RED; RED: %d, BLACK: %d, STATE: %d, #: %d\n", graph->red_vertex_count, graph->black_vertex_count, graph->state, graph->state_count);
            }
            pthread_mutex_unlock(&graph->lock);
            pthread_mutex_unlock(&graph->color_lock);
            break;
        case BLACK:
            pthread_mutex_lock(&graph->lock);
            graph->red_locked = 1;
            graph->black_locked = 0;
            pthread_mutex_unlock(&graph->lock);
            pthread_cond_wait(&graph->black_fire, &graph->color_lock);
            pthread_mutex_lock(&graph->lock);
            if (graph->black_vertex_count == 0)
            {
                /** TODO: REAP BLACK **/
                graph->state = PRINT;
                graph->previous_color = BLACK;
                graph->print_flag = 1;
                //fprintf(stderr, "WAS BLACK; RED: %d, BLACK: %d, STATE: %d, #: %d\n", graph->red_vertex_count, graph->black_vertex_count, graph->state, graph->state_count);
            }
            pthread_mutex_unlock(&graph->lock);
            pthread_mutex_unlock(&graph->color_lock);
            break;
        case PRINT:
            pthread_mutex_lock(&graph->lock);
            graph->red_locked = 1;
            graph->black_locked = 1;
            pthread_mutex_unlock(&graph->lock);
            if (graph->print_flag == 1)
            {
                if (process_requests(graph) < 0)
                {
                    graph->state = TERMINATE;
                    return -1;
                }
                print_graph(graph);
                pthread_mutex_lock(&graph->lock);
                graph->state_count++;
                if (graph->previous_color == RED)
                    graph->state = BLACK;
                else
                    graph->state = RED;
                if (graph->num_vertices == 0)
                {
                    graph->state = TERMINATE;
                    graph->red_locked = 0;
                    graph->black_locked = 0;
                }
                if (graph->state == TERMINATE)
                    printf("KILLING TIME\n");
                pthread_mutex_unlock(&graph->lock);
            }
            graph->print_flag = 0;
            break;
        default:
            pthread_exit(NULL);
            return -1;
        }
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
        if (graph)
        {
            printf("?\n");
            pthread_mutex_lock(&graph->lock);
            if (color == RED)
            {
                --(graph->num_vertices);
                if (graph->red_vertex_count <= 0)
                    pthread_cond_signal(&graph->red_fire);
            }
            else if (color == BLACK)
            {
                --(graph->num_vertices);
                if (graph->black_vertex_count <= 0)
                    pthread_cond_signal(&graph->black_fire);
            }
            pthread_mutex_unlock(&graph->lock);
        }
        if (graph && vertex) return 0;
        return -1;
    }

    enum STATES flip_color = BLACK;
    //pthread_mutex_lock(&vertex->lock);

    if (color == RED)
    {
        pthread_mutex_lock(&graph->lock);
        graph->red_vertex_count++;
        pthread_mutex_unlock(&graph->lock);
        while (graph->red_locked)
        {
        }
        pthread_mutex_lock(&vertex->lock);
    }
    else if (color == BLACK)
    {
        pthread_mutex_lock(&graph->lock);
        graph->black_vertex_count++;
        pthread_mutex_unlock(&graph->lock);
        flip_color = RED;
        while (graph->black_locked)
        {
        }
        pthread_mutex_lock(&vertex->lock);
    }
    else
    {
        //pthread_mutex_unlock(&vertex->lock);
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
    //NEED TO LOCK RED OR BLACK IN GRAPH
    //pthread_mutex_lock(&vertex->lock);

    if (graph->state == TERMINATE)
    {
        vertex->is_active = 0;
        pthread_mutex_lock(&graph->lock);

        if (color == RED)
        {
            graph->red_vertex_count--;
            --(graph->num_vertices);
            if (graph->red_vertex_count == 0)
                pthread_cond_signal(&graph->red_fire);
        }
        else
        {
            graph->black_vertex_count--;
            --(graph->num_vertices);
            if (graph->black_vertex_count == 0)
                pthread_cond_signal(&graph->black_fire);
        }
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

    (vertex->f)(graph, args);

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
        if ((edge->f)(args->edge_argv))
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
                    {
                        graph->red_vertex_count--;
                        if (graph->red_vertex_count == 0)
                            pthread_cond_signal(&graph->red_fire);
                    }
                    else
                    {
                        graph->black_vertex_count--;
                        if (graph->black_vertex_count == 0)
                            pthread_cond_signal(&graph->black_fire);
                    }
                    //NEED TO SIGNAL P in RUN
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
                pthread_mutex_lock(&graph->lock);
                ++(graph->num_vertices);
                pthread_mutex_unlock(&graph->lock);
                edge->b->is_active = 1;
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
    {
        graph->red_vertex_count--;
        --(graph->num_vertices);
        if (graph->red_vertex_count == 0)
            pthread_cond_signal(&graph->red_fire);
    }
    else
    {
        graph->black_vertex_count--;
        --(graph->num_vertices);
        if (graph->black_vertex_count == 0)
            pthread_cond_signal(&graph->black_fire);
    }

    //NEED TO SIGNAL P in RUN
    pthread_mutex_unlock(&graph->lock);
    vertex->is_active = 0;
    pthread_mutex_unlock(&vertex->lock);
    //fprintf(stderr, "UNLOCKED\n");

    int iloop_b = 1;
    if (next_vertex == vertex)
        iloop_b = iloop + 1;
    if (graph->context == NONE && next_vertex != NULL)
    {
        next_vertex->is_active = 1;
        ++(graph->num_vertices);
        return fire(graph, next_vertex, args, flip_color, iloop_b);
    }
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
    //pthread_join(graph->thread, NULL);
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

    free(vargp);
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
    argv->args->vertex_argv = NULL;
    if (args->vertex_argv && args->vertex_size > 0)
    {
        argv->args->vertex_argv = malloc(sizeof(args->vertex_size));
        if (!argv->args->vertex_argv)
        {
            free(argv->args);
            free(argv);
            return -1;
        }
        memcpy(argv->args->vertex_argv, args->vertex_argv, args->vertex_size);
    }
    argv->args->edge_argv = NULL;
    if (args->edge_argv && args->edge_size > 0)
    {
        argv->args->edge_argv = malloc(sizeof(args->edge_size));
        if (!argv->args->vertex_argv)
        {
            free(argv->args->vertex_argv);
            free(argv->args);
            free(argv);
            return -1;
        }
        memcpy(argv->args->edge_argv, args->edge_argv, args->edge_size);
    }

    argv->args->vertex_size = args->vertex_size;
    argv->args->edge_size = args->edge_size;
    argv->graph = graph;
    argv->vertex = vertex;
    argv->color = color;
    argv->iloop = iloop;
    int thread_result = 0;
    int thread_attempts = 0;
    pthread_t thread;
create_switch_threads:
    thread_result = pthread_create(&thread, NULL, fire_pthread, argv);
    ++thread_attempts;
    if (thread_result != 0)
    {
        if (errno != EAGAIN)
        {
            perror("Creating initial Threads: ");
            free(argv);
            return -1;
        }
        else
        {
            switch (graph->mem_option)
            {
            case CONTINUE:
                free(argv);
                return -1;
            case WAIT:
                if (thread_attempts > MAX_ATTEMPTS)
                {
                    fprintf(stderr, "Max Threads Attempts Hit\n");
                    free(argv);
                    return -1;
                }
                sleep(THREAD_ATTEMPT_SLEEP);
                errno = 0;
                goto create_switch_threads;
            case ABORT:
                fprintf(stderr, "Failed to Create Threads\n");
                free(argv);
                return -1;
            }
        }
    }
    pthread_detach(thread);
    //free(argv);

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
