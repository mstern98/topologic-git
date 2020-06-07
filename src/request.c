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
        if(!f) {
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

int submit_request(struct graph *graph, struct request *request)
{
    if (!graph || !request)
        return -1;
    int retval = 0;
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
    pthread_mutex_unlock(&graph->lock);
    return retval;
}

void procces_request(struct request *request)
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
        struct destroy_edge_id_request *args = (struct destroy_edge_id_request *) request->args;
        remove_edge_id(args->a, args->id);
        break;
    }
    case DESTROY_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *) request->args;
        remove_edge(args->a, args->b);
        break;
    }
    case DESTROY_BI_EDGE:
    {
        struct destroy_edge_request *args = (struct destroy_edge_request *) request->args;
        remove_bi_edge(args->a, args->b);
        break;
    }
    case DESTROY_VERTEX:
    {
        struct destroy_vertex_request *args = (struct destroy_vertex_request *) request->args;
        remove_vertex(args->graph, args->vertex);
        break;
    }
    case CREAT_VERTEX:
    {
        struct vertex_request *args = (struct vertex_request *)request->args;
        create_vertex(args->graph, args->f, args->id, args->glblc, args->glbl);
        break;
    }
    case CREAT_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        create_edge(args->a, args->b, args->f, args->glblc, args->glbl);
        break;
    }
    case CREAT_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        create_bi_edge(args->a, args->b, args->f, args->glblc, args->glbl);
        break;
    }
    case MOD_VERTEX:
    {
        struct mod_vertex_request *args = (struct mod_vertex_request *)request->args;
        modify_vertex(args->vertex, args->f, args->glblc, args->glbl);
        break;
    }
    case MOD_EDGE_VARS:
    {
        struct mod_edge_vars_request *args = (struct mod_edge_vars_request *)request->args;
        modify_shared_edge_vars(args->vertex, args->edgec, args->edge_vars);
        break;
    }
    case MOD_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        modify_edge(args->a, args->b, args->f, args->glblc, args->glbl);
        break;
    }
    case MOD_BI_EDGE:
    {
        struct edge_request *args = (struct edge_request *)request->args;
        modify_bi_edge(args->a, args->b, args->f, args->glblc, args->glbl);
        break;
    }
    default:
        return;
    }
}

void process_requests(struct graph *graph)
{
    if (!graph)
        return;

    pthread_mutex_lock(&graph->lock);

    struct request *req = NULL;
    while ((req = (struct request *)pop(graph->modify)) != NULL)
        procces_request(req);
    while ((req = (struct request *)pop(graph->remove_edges)) != NULL)
        procces_request(req);
    while ((req = (struct request *)pop(graph->remove_vertices)) != NULL)
        procces_request(req);

    pthread_mutex_unlock(&graph->lock);
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