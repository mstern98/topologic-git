// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

void sleep_ms(int milliseconds)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

int start_set(struct graph *graph, int id[], int num_vertices)
{
    topologic_debug("%s;id: %p;num_vertices: %d", "start_set", id, num_vertices);
    if (!graph)
    {
        topologic_debug("%s;%s;%d", "start_set", "invalid graph", -1);
        return -1;
    }
    if (!id)
    {
        topologic_debug("%s;%s;%d", "start_set", "no id", -1);
        return -1;
    }
    if (num_vertices <= 0 || (graph->context == SINGLE && num_vertices > 1))
    {
        topologic_debug("%s;%s;%d", "start_set", "invalid number of vertices", -1);
        return -1;
    }

    while (pop(graph->start) != NULL)
    {
    }

    int i = 0;
    for (; i < num_vertices; i++)
    {
        struct vertex *v = (struct vertex *)find(graph->vertices, id[i]);
        if (!v || push(graph->start, v) < 0)
        {
            if (v)
                topologic_debug("%s;%s_%p_%s_%d;%d", "start_set", "could not push vertex", v, "id: ", v->id, -1);
            else
                topologic_debug("%s;%s;%d", "start_set", "vertex invalid", -1);
            /** Handle errors **/
            /**Given vertx failed, so at this point, free the vertices and leave**/
            while (pop(graph->start) != NULL)
            {
            }
            //destroy_graph(graph);
            return -1;
        }
    }
    topologic_debug("%s;%s;%d", "start_set", "succeeded", 0);
    return 0;
}

int run_single(struct graph *graph, struct vertex_result **init_vertex_args)
{
    topologic_debug("%s;graph: %p;init_vertex_args: %p", "run_single", graph, init_vertex_args);
    if (!graph || graph->context != SINGLE || graph->start->length > 1 || graph->start->length == 0)
    {
        topologic_debug("%s;%s;%d", "run_single", "failed to run", -1);
        return -1;
    }
    int successor = 0;

    struct vertex *vertex = (struct vertex *)pop(graph->start);
    if (!vertex)
    {
        topologic_debug("%s;%s;%d", "run_single", "no vertex", -1);
        return -1;
    }
    topologic_debug("%s;%s;%p", "run_single", "initial vertex", vertex);
    struct edge *edge = NULL;

    struct vertex_result *args = init_vertex_args[0];
    free(init_vertex_args);
    init_vertex_args = NULL;
    struct stack *edges = init_stack();
    if (!edges)
    {
        topologic_debug("%s;%s;%d", "run_single", "no edge", -1);
        return -1;
    }

    int ret = 0;
    int iloop = 0;
    while (graph->state != TERMINATE)
    {
        vertex->is_active = 1;
        graph->num_vertices = 1;
        print_graph(graph);
        preorder(vertex->edge_tree, edges);
        pthread_mutex_lock(&graph->lock);
        while (graph->pause)
        {
            pthread_cond_wait(&graph->pause_cond, &graph->lock);
        }
        pthread_mutex_unlock(&graph->lock);
        (vertex->f)(graph, args, vertex->glbl, vertex->shared->vertex_data);
        while ((edge = (struct edge *)pop(edges)) != NULL)
        {
            if (successor == 0 && (edge->f)(args->edge_argv, edge->glbl, edge->a_vars, edge->b_vars))
            {
                topologic_debug("%s;%s;%p", "run_single", "next vertex", edge->b);
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
            topologic_debug("%s;%s;%d", "run_single", "process_request failed", -1);
            graph->state = TERMINATE;
            ret = -1;
            break;
        }
        ++(graph->state_count);
        if (successor == 0)
        {
            topologic_debug("%s;%s;%d", "run_single", "no successor", 0);
            graph->state = TERMINATE;
            graph->num_vertices = 0;
            print_graph(graph);
        }
        else
            successor = 0;
        if ((graph->max_state_changes != -1 && graph->state_count >= graph->max_state_changes) ||
            (graph->max_loop != -1 && iloop >= graph->max_loop))
        {
            graph->state = TERMINATE;
            graph->num_vertices = 0;
            print_graph(graph);
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
    topologic_debug("%s;%s;%d", "run_single", "finished", ret);
    return ret;
}

int run(struct graph *graph, struct vertex_result **init_vertex_args)
{
    topologic_debug("%s;graph %p;init_vertex_args %p", "run", graph, init_vertex_args);
    if (!graph || !graph->start || graph->state == TERMINATE)
    {
        topologic_debug("%s;%s;%d", "run", "invalid graph", -1);
        return -1;
    }
    if (graph->context == SINGLE)
        return run_single(graph, init_vertex_args);

    int success = 0, v_index = 0;
    struct vertex *v = NULL;
    while ((v = (struct vertex *)pop(graph->start)))
    {
        if (!success)
            success = 1;

        struct fireable *argv = (struct fireable *)malloc(sizeof(struct fireable));
        if (!argv)
        {
            success = 0;
            topologic_debug("%s;%s;%d", "run", "Fireable failed to malloc", -1);
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
                topologic_debug("%s;%s", "switch_vertex", "Couldn't create intial threads");
                success = -1;
                break;
            }
            else
            {
                switch (graph->mem_option)
                {
                case CONTINUE:
                    topologic_debug("%s;%s", "switch_vertex", "Continue");
                    break;
                case WAIT:
                    if (thread_attempts > MAX_ATTEMPTS)
                    {
                        success = -1;
                        topologic_debug("%s;%s", "run", "Max Thread Attempts Hit");
                        break;
                    }
                    sleep(THREAD_ATTEMPT_SLEEP);
                    errno = 0;
                    goto create_start_threads;
                case ABORT:
                    success = -1;
                    topologic_debug("%s;%s", "run", "Failed to create threads");
                    break;
                }
            }
        }
        pthread_detach(thread);
        v->is_active = 1;
        ++(graph->num_vertices);
        ++(graph->red_vertex_count);
        ++v_index;

        argv = NULL;
    }

    if (!success)
    {
        topologic_debug("%s;%s;%d", "run", "Failed to create threads", -1);
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
            topologic_debug("%s;%s;", "run", "RED");
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
            }
            pthread_mutex_unlock(&graph->lock);
            pthread_mutex_unlock(&graph->color_lock);
            break;
        case BLACK:
            topologic_debug("%s;%s;", "run", "BLACK");
            pthread_mutex_lock(&graph->lock);
            graph->red_locked = 1;
            graph->black_locked = 0;
            pthread_mutex_unlock(&graph->lock);
            pthread_cond_wait(&graph->black_fire, &graph->color_lock);
            pthread_mutex_lock(&graph->lock);
            if (graph->black_vertex_count == 0)
            {
                graph->state = PRINT;
                graph->previous_color = BLACK;
                graph->print_flag = 1;
            }
            pthread_mutex_unlock(&graph->lock);
            pthread_mutex_unlock(&graph->color_lock);
            break;
        case PRINT:
            topologic_debug("%s;%s;", "run", "PRINT");
            pthread_mutex_lock(&graph->lock);
            graph->red_locked = 1;
            graph->black_locked = 1;
            pthread_mutex_unlock(&graph->lock);
            if (graph->print_flag == 1)
            {
                if (process_requests(graph) < 0)
                {
                    graph->state = TERMINATE;
                    topologic_debug("%s;%s;%d", "run", "process_request failed", -1);
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
                    topologic_debug("%s;%s;", "run", "no more vertices");
                    graph->state = TERMINATE;
                    graph->red_locked = 0;
                    graph->black_locked = 0;
                }
                pthread_mutex_unlock(&graph->lock);
            }
            graph->print_flag = 0;
            break;
        default:
            topologic_debug("%s;%s;%d", "run", "INVALID STATE", -1);
            pthread_exit(NULL);
            return -1;
        }
    }
    topologic_debug("%s;%s;%d", "run", "finished", 0);
    return 0;
}

int fire(struct graph *graph, struct vertex *vertex, struct vertex_result *args, enum STATES color, int iloop)
{
    topologic_debug("%s;graph %p;vertex %p;args %p;color %d;iloop %d", "fire", graph, vertex, args, color, iloop);
    int retval = 0;
    struct vertex *next_vertex = NULL;

    if (!graph || !vertex)
    {
        topologic_debug("%s;%s;%d", "fire", "invalid args", -1);
        retval = -1;
        goto clean_fire;
    }

    enum STATES flip_color = BLACK;

    if (color == RED)
    {
        while (graph->red_locked)
        {
        }
    }
    else if (color == BLACK)
    {
        flip_color = RED;
        while (graph->black_locked)
        {
        }
    }
    else
    {
        topologic_debug("%s;%s;%d", "fire", "invalid state", -1);
        retval = -1;
        goto clean_fire;
    }
    
    pthread_mutex_lock(&vertex->lock);

    if (graph->state == TERMINATE)
    {
        topologic_debug("%s;%s;%d", "fire", "terminate", -1);
        retval = -1;
        goto exit_fire;
    }

    if (graph->max_loop != -1 && iloop >= graph->max_loop)
    {
        topologic_debug("%s;%s;%d", "fire", "max loop hit", 0);
        goto exit_fire;
    }

    (vertex->f)(graph, args, vertex->glbl, vertex->shared->vertex_data);

    if (graph->max_state_changes != -1 && graph->state_count + 1 >= graph->max_state_changes)
        goto exit_fire;

    struct stack *edges = init_stack();
    preorder(vertex->edge_tree, edges);
    struct edge *edge = NULL;
    while ((edge = (struct edge *)pop(edges)) != NULL)
    {
        if (edge->edge_type == BI_EDGE)
        {
            pthread_mutex_lock(&edge->bi_edge_lock);
        }
        if ((edge->f)(args->edge_argv, edge->glbl, edge->a_vars, edge->b_vars))
        {
            if (edge->edge_type == BI_EDGE)
            {
                pthread_mutex_unlock(&edge->bi_edge_lock);
            }
            if (graph->context == SWITCH || graph->context == SWITCH_UNSAFE)
            {
                int iloop_b = 1;
                if (edge->b == vertex)
                    iloop_b = iloop + 1;
                if (switch_vertex(graph, edge->b, args, flip_color, iloop_b) < 0)
                {
                    topologic_debug("%s;%s;%d", "fire", "failed to switch", -1);
                    retval = -1;
                    destroy_stack(edges);
                    edges = NULL;
                    edge = NULL;
                    goto exit_fire;
                }
                pthread_mutex_lock(&graph->lock);
                ++(graph->num_vertices);
                if (flip_color == RED)
                    ++(graph->red_vertex_count);
                else if (flip_color == BLACK)
                    ++(graph->black_vertex_count);
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

exit_fire:
    pthread_mutex_lock(&graph->lock);
    if (color == RED)
    {
        --(graph->red_vertex_count);
        --(graph->num_vertices);
        if (graph->red_vertex_count == 0)
            pthread_cond_signal(&graph->red_fire);
    }
    else
    {
        --(graph->black_vertex_count);
        --(graph->num_vertices);
        if (graph->black_vertex_count == 0)
            pthread_cond_signal(&graph->black_fire);
    }
    pthread_mutex_unlock(&graph->lock);
    vertex->is_active = 0;
    pthread_mutex_unlock(&vertex->lock);

    int iloop_b = 1;
    if (next_vertex == vertex)
        iloop_b = iloop + 1;
    if (graph->context == NONE && next_vertex != NULL)
    {
        next_vertex->is_active = 1;
        pthread_mutex_lock(&graph->lock);
        ++(graph->num_vertices);
        if (flip_color == RED)
            ++(graph->red_vertex_count);
        else if (flip_color == BLACK)
            ++(graph->black_vertex_count);
        pthread_mutex_unlock(&graph->lock);
        topologic_debug("%s;%s;%p", "fire", "firing next vertex", next_vertex);
        sleep_ms(PTHREAD_SLEEP_TIME);
        return fire(graph, next_vertex, args, flip_color, iloop_b);
    }
clean_fire:
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
    topologic_debug("%s;%s;%d", "fire", "finished", retval);
    return retval;
}

void *fire_pthread(void *vargp)
{
    topologic_debug("%s;%p", "fire_pthread", vargp);
    if (!vargp)
    {
        topologic_debug("%s;%s;%d", "fire", "invalid args", -1);
        return (void *)(intptr_t)-1;
    }
    struct fireable *fireable = (struct fireable *)vargp;

    struct graph *graph = fireable->graph;
    struct vertex *v = fireable->vertex;
    struct vertex_result *args = fireable->args;
    enum STATES color = fireable->color;
    int iloop = fireable->iloop;

    destroy_fireable((struct fireable *) vargp);
    sleep_ms(PTHREAD_SLEEP_TIME);
    int ret_val = fire(graph, v, args, color, iloop);
    topologic_debug("%s;%s;%d", "fire_pthread", "finished", ret_val);
    pthread_exit((void *)(intptr_t)ret_val);
    return (void *)(intptr_t)ret_val;
}

int switch_vertex(struct graph *graph, struct vertex *vertex, struct vertex_result *args, enum STATES color, int iloop)
{
    topologic_debug("%s;graph %p;vertex %p;args %p;color %d;iloop %d", "switch_vertex", graph, vertex, args, color, iloop);
    struct fireable *argv = create_fireable(graph, vertex, args, color, iloop);
    if (!argv) 
    {
        topologic_debug("%s;%s;%d", "switch_vertex", "failed to create fireable", -1);
        return -1;
    }
    
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
            topologic_debug("%s;%s;%d", "switch_vertex", "Couldn't create threads", -1);
            destroy_fireable(argv);
            return -1;
        }
        else
        {
            switch (graph->mem_option)
            {
            case CONTINUE:
                topologic_debug("%s;%s;%d", "switch_vertex", "Continue", -1);
                destroy_fireable(argv);
                return -1;
            case WAIT:
                if (thread_attempts > MAX_ATTEMPTS)
                {
                    topologic_debug("%s;%s;%d", "switch_vertex", "Max Thread Attempts Hit", -1);
                    destroy_fireable(argv);
                    return -1;
                }
                sleep(THREAD_ATTEMPT_SLEEP);
                errno = 0;
                goto create_switch_threads;
            case ABORT:
                topologic_debug("%s;%s;%d", "switch_vertex", "Failed to Create Thread", -1);
                destroy_fireable(argv);
                return -1;
            }
        }
    }
    pthread_detach(thread);
    topologic_debug("%s;%s;%d", "switch_vertex", "finished", 0);
    return 0;
}

int pause_graph(struct graph *graph)
{
    topologic_debug("%s;graph %p;", "pause_graph", graph);
    if (!graph)
    {
        topologic_debug("%s;%s;%d", "pause_graph", "invalid graph", -1);
        return -1;
    }
    if (graph->pause == 1)
    {
        topologic_debug("%s;%s;%d", "pause_graph", "graph already paused", -1);
        return -1;
    }

    pthread_mutex_lock(&graph->lock);
    graph->pause = 0;
    pthread_cond_signal(&graph->pause_cond);
    pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s;%d", "pause_graph", "succeeded", 0);
    return 0;
}

int resume_graph(struct graph *graph)
{
    topologic_debug("%s;graph %p;", "resume_graph", graph);

    if (!graph)
    {
        topologic_debug("%s;%s;%d", "resume_graph", "invalid graph", -1);
        return -1;
    }
    if (graph->pause == 0)
    {
        topologic_debug("%s;%s;%d", "resume_graph", "graph already running", -1);
        return -1;
    }

    pthread_mutex_lock(&graph->lock);
    graph->pause = 1;
    pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s;%d", "resume_graph", "succeeded", 0);
    return 0;
}
