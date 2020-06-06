#include "../include/topologic.h"

struct request *create_request(enum REQUESTS request, void *args, void (*f)(void *), int argc)
{
    struct request *req = malloc(sizeof(struct request));
    if (!req)
        return NULL;
    req->args = args;
    req->argc = argc;
    req->f = f;
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
    case MODIFY:
        retval = push(graph->modify, (void *)request);
        break;
    case DESTROY_EDGE:
        retval = push(graph->remove_edges, (void *)request);
        break;
    case DESTROY_VERTEX:
        retval = push(graph->remove_vertices, (void *)request);
        break;
    default:
        retval = -1;
        break;
    }
    pthread_mutex_unlock(&graph->lock);
    return retval;
}

void process_requests(struct graph *graph)
{
    if (!graph)
        return;

    pthread_mutex_lock(&graph->lock);

    struct request *req = NULL;
    while ((req = (struct request *)pop(graph->modify)) != NULL)
        (req->f)(req->args);
    while ((req = (struct request *)pop(graph->remove_edges)) != NULL)
        (req->f)(req->args);
    while ((req = (struct request *)pop(graph->remove_vertices)) != NULL)
        (req->f)(req->args);

    pthread_mutex_unlock(&graph->lock);
}

int destroy_request(struct request *request)
{
    if (!request)
        return -1;
    
    free(request->args);
    request->args = NULL;
    request->argc = 0;
    request->request = 0;
    return 0;
}