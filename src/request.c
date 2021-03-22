// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct request *create_request(enum REQUESTS request, void *args, void (*f)(void *))
{
    topologic_debug("%s;request %d;args %p;f %p", "create_request", request, args, f);
    struct request *req = (struct request *)malloc(sizeof(struct request));
    if (!req)
    {
        topologic_debug("%s;%s;%p", "create_request", "failed to malloc request", (void *) NULL);
        return NULL;
    }
    req->args = args;

    switch (request)
    {
    case GENERIC:
        if (!f)
        {
            free(req);
            req = NULL;
            topologic_debug("%s;%s;%p", "create_request", "NULL function", (void *) NULL);
            return NULL;
        }
        req->f = f;
        break;
    case DESTROY_EDGE:
    case DESTROY_BI_EDGE:
    case DESTROY_VERTEX:
    case CREAT_VERTEX:
    case CREAT_EDGE:
    case CREAT_BI_EDGE:
    case MOD_VERTEX:
    case MOD_EDGE_VARS:
    case MOD_EDGE:
    case MOD_BI_EDGE:
    case DESTROY_EDGE_BY_ID:
    case DESTROY_VERTEX_BY_ID:
        req->f = NULL;
        break;
    default:
        topologic_debug("%s;%s;%p", "create_request", "invalid request", (void *) NULL);
        free(req);
        req = NULL;
        return NULL;
    }
    req->request = request;
    topologic_debug("%s;%s;%p", "create_request", "success", req);
    return req;
}

int submit_request(struct graph *graph, struct request *request)
{
    topologic_debug("%s;graph %p;request %p", "submit_request", graph, request);
    if (!request || !graph)
    {
        topologic_debug("%s;%s;%d", "submit_request", "invalid args", -1);
        return -1;
    }
    int retval = 0;
    enum CONTEXT context = graph->context;
    if (context != SINGLE)
        pthread_mutex_lock(&graph->lock);
    switch (request->request)
    {
    case DESTROY_EDGE:
    case DESTROY_BI_EDGE:
    case DESTROY_EDGE_BY_ID:
        retval = push(graph->remove_edges, (void *)request);
        break;
    case DESTROY_VERTEX:
    case DESTROY_VERTEX_BY_ID:
        retval = push(graph->remove_vertices, (void *)request);
        break;
    default:
        retval = push(graph->modify, (void *)request);
        break;
    }
    if (context != SINGLE)
        pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s;%d", "submit_request", "finished", retval);
    return retval;
}

int procces_request(struct request *request)
{
    topologic_debug("%s;request %p", "process_request", request);
    if (!request)
    {
        topologic_debug("%s;%s;%d", "process_request", "invalid args", -1);
        return -1;
    }
    switch (request->request)
    {
    case GENERIC:
    {
        (request->f)(request->args);
        break;
    }
    case DESTROY_EDGE_BY_ID:
    {
        struct destroy_edge_id_request *args = (struct destroy_edge_id_request *)request->args;
        int ret = remove_edge_id(args->a, args->id);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = DESTROY_EDGE_BY_ID;
            return -1;
        }
        break;
    }
    case DESTROY_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *)request->args;
        int ret = remove_edge(args->a, args->b);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = DESTROY_EDGE;
            return -1;
        }
        break;
    }
    case DESTROY_BI_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *)request->args;
        int ret = remove_bi_edge(args->a, args->b);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = DESTROY_BI_EDGE;
            return -1;
        }
        break;
    }
    case DESTROY_VERTEX:
    {
        struct destroy_vertex_request *args = (struct destroy_vertex_request *)request->args;
        int ret = remove_vertex(args->graph, args->vertex);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = DESTROY_VERTEX;
            return -1;
        }
        break;
    }
    case DESTROY_VERTEX_BY_ID:
    {
        struct destroy_vertex_id_request *args = (struct destroy_vertex_id_request *)request->args;
        int ret = remove_vertex_id(args->graph, args->id);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = DESTROY_VERTEX_BY_ID;
            return -1;
        }
        break;
    }
    case CREAT_VERTEX:
    {
        struct vertex_request *args = (struct vertex_request *)request->args;
        struct vertex *ret = create_vertex(args->graph, args->f, args->id, args->glbl);
        topologic_debug("%s;request %d;%p", "process_request", request->request, ret);
        if (!ret)
        {
            errno = CREAT_VERTEX;
            return -1;
        }
        break;
    }
    case CREAT_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        struct edge *ret = create_edge(args->a, args->b, args->f, args->glbl);
        topologic_debug("%s;request %d;%p", "process_request", request->request, ret);
        if (!ret)
        {
            errno = CREAT_EDGE;
            return -1;
        }
        break;
    }
    case CREAT_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = create_bi_edge(args->a, args->b, args->f, args->glbl, NULL, NULL);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = CREAT_BI_EDGE;
            return -1;
        }
        break;
    }
    case MOD_VERTEX:
    {
        struct mod_vertex_request *args = (struct mod_vertex_request *)request->args;
        int ret = modify_vertex(args->vertex, args->f, args->glbl);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = MOD_VERTEX;
            return -1;
        }
        break;
    }
    case MOD_EDGE_VARS:
    {
        struct mod_edge_vars_request *args = (struct mod_edge_vars_request *)request->args;
        int ret = modify_shared_edge_vars(args->vertex, args->edge_vars);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = MOD_EDGE_VARS;
            return -1;
        }
        break;
    }
    case MOD_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = modify_edge(args->a, args->b, args->f, args->glbl);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = MOD_EDGE;
            return -1;
        }
        break;
    }
    case MOD_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = modify_bi_edge(args->a, args->b, args->f, args->glbl);
        topologic_debug("%s;request %d;%d", "process_request", request->request, ret);
        if (ret < 0)
        {
            errno = MOD_BI_EDGE;
            return -1;
        }
        break;
    }
    default:
        return 0;
    }
    free(request->args);
    request->args = NULL;
    free(request);
    request = NULL;
    return 0;
}

int process_requests(struct graph *graph)
{
    topologic_debug("%s;graph %p", "process_requests", graph);
    if (!graph)
    {
        topologic_debug("%s;%s;%d", "process_requests", "invalid args", -1);
        return -1;
    }

    if (graph->context != SINGLE)
        pthread_mutex_lock(&graph->lock);

    struct request *req = NULL;
    while ((req = (struct request *)pop(graph->modify)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(req) < 0 && graph->request_flag == NO_FAIL_REQUEST)
        {
            topologic_debug("%s;%s;%d", "process_requests", "failed to process general requests", -1);
            return -1;
        }
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }
    while ((req = (struct request *)pop(graph->remove_edges)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(req) < 0 && graph->request_flag == NO_FAIL_REQUEST)
        {
            topologic_debug("%s;%s;%d", "process_requests", "failed to remove edges", -1);
            return -1;
        }
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }
    while ((req = (struct request *)pop(graph->remove_vertices)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(req) < 0 && graph->request_flag == NO_FAIL_REQUEST)
        {
            topologic_debug("%s;%s;%d", "process_requests", "failed to remove vertices", -1);
            return -1;
        }
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }

    if (graph->context != SINGLE)
        pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s;%d", "process_requests", "success", 0);
    return 0;
}

int destroy_request(struct request *request)
{
    topologic_debug("%s;request %p", "destroy_request", request);
    if (!request)
    {
        topologic_debug("%s;%s;%d", "destroy_request", "invalid args", -1);
        return -1;
    }

    free(request->args);
    request->args = NULL;
    request->request = (enum REQUESTS) 0;
    request->f = NULL;
    free(request);
    request = NULL;
    topologic_debug("%s;%s;%d", "destroy_request", "success", 0);
    return 0;
}
