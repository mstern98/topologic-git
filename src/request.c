// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct request *create_request(enum REQUESTS request, void *args, void (*f)(void *))
{
    struct request *req = malloc(sizeof(struct request));
    if (!req)
        return NULL;
    req->args = args;

    switch (request)
    {
    case GENERIC:
        if (!f)
        {
            free(req);
            req = NULL;
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
        req->f = NULL;
        break;
    default:
        free(req);
        req = NULL;
        return NULL;
    }
    req->request = request;
    return req;
}

int submit_request(struct graph* graph, struct request *request)
{
    if (!request)
        return -1;
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
        retval = push(graph->remove_vertices, (void *)request);
        break;
    default:
        retval = push(graph->modify, (void *)request);
        break;
    }
    if (context != SINGLE)
        pthread_mutex_unlock(&graph->lock);
    return retval;
}

int procces_request(struct graph *graph, struct request *request)
{
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
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Destroy Edge Id Request (%p)\n", request);
            errno = DESTROY_EDGE_BY_ID;
            return -1;
        }
        break;
    }
    case DESTROY_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *)request->args;
        int ret = remove_edge(args->a, args->b);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Destroy Edge Request (%p)\n", request);
            errno = DESTROY_EDGE;
            return -1;
        }
        break;
    }
    case DESTROY_BI_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *)request->args;
        int ret = remove_bi_edge(args->a, args->b);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Destroy Bi Edge Request (%p)\n", request);
            errno = DESTROY_BI_EDGE;
            return -1;
        }
        break;
    }
    case DESTROY_VERTEX:
    {
        struct destroy_vertex_request *args = (struct destroy_vertex_request *)request->args;
        int ret = remove_vertex(args->graph, args->vertex);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Destroy Vertex Request (%p)\n", request);
            errno = DESTROY_VERTEX;
            return -1;
        }
        break;
    }
    case CREAT_VERTEX:
    {
        struct vertex_request *args = (struct vertex_request *)request->args;
        struct vertex *ret = create_vertex(args->graph, args->f, args->id, args->glbl);
        if (!ret)
        {
            //fprintf(stderr, "Failed Create Vertex Request (%p)\n", request);
            errno = CREAT_VERTEX;
            return -1;
        }
        break;
    }
    case CREAT_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        struct edge *ret = create_edge(args->a, args->b, args->f, args->glbl);
        if (!ret)
        {
            //fprintf(stderr, "Failed Create Edge Request (%p)\n", request);
            errno = CREAT_EDGE;
            return -1;
        }
        break;
    }
    case CREAT_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = create_bi_edge(args->a, args->b, args->f, args->glbl, NULL, NULL);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Destroy Edge Id Request: %d (%p)\n", ret, request);
            errno = CREAT_BI_EDGE;
            return -1;
        }
        break;
    }
    case MOD_VERTEX:
    {
        struct mod_vertex_request *args = (struct mod_vertex_request *)request->args;
        int ret = modify_vertex(args->vertex, args->f, args->glbl);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Modify Vertex Request (%p)\n", request);
            errno = MOD_VERTEX;
            return -1;
        }
        break;
    }
    case MOD_EDGE_VARS:
    {
        struct mod_edge_vars_request *args = (struct mod_edge_vars_request *)request->args;
        int ret = modify_shared_edge_vars(args->vertex, args->edge_vars);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Modify Edge Vars Request (%p)\n", request);
            errno = MOD_EDGE_VARS;
            return -1;
        }
        break;
    }
    case MOD_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = modify_edge(args->a, args->b, args->f, args->glbl);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Modify Edge Request (%p)\n", request);
            errno = MOD_EDGE;
            return -1;
        }
        break;
    }
    case MOD_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        int ret = modify_bi_edge(args->a, args->b, args->f, args->glbl);
        if (ret < 0)
        {
            //fprintf(stderr, "Failed Modify Bi Edge Request: %d (%p)\n", ret, request);
            errno = MOD_BI_EDGE;
            return -1;
        }
        break;
    }
    default:
        return 0;
    }
    return 0;
}

int process_requests(struct graph *graph)
{
    if (!graph)
        return -1;

    if (graph->context != SINGLE)
        pthread_mutex_lock(&graph->lock);

    struct request *req = NULL;
    while ((req = (struct request *)pop(graph->modify)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(graph,req) < 0)
            return -1;
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }
    while ((req = (struct request *)pop(graph->remove_edges)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(graph,req) < 0)
            return -1;
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }
    while ((req = (struct request *)pop(graph->remove_vertices)) != NULL)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        if (procces_request(graph,req) < 0)
            return -1;
        if (graph->context != SINGLE)
            pthread_mutex_lock(&graph->lock);
    }

    if (graph->context != SINGLE)
        pthread_mutex_unlock(&graph->lock);
    return 0;
}

int destroy_request(struct request *request)
{
    if (!request)
        return -1;

    free(request->args);
    request->args = NULL;
    request->request = 0;
    return 0;
}
