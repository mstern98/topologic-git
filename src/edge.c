// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

struct edge *create_edge(struct vertex *a, struct vertex *b, int (*f)(void *, void *, const void *const, const void *const), void *glbl)
{
    topologic_debug("%s;a %p;b %p;f %p;glbl %p", "create_edge", a, b, f, glbl);
    if (!a || !b)
    {
        topologic_debug("%s;%s;%p", "create_edge", "invalid vertices", (void *) NULL);
        return NULL;
    }
    if (!f)
    {
        topologic_debug("%s;%s;%p", "create_edge", "invalid function", (void *) NULL);
        return NULL;
    }

    enum CONTEXT context = a->context;

    if (context != SINGLE)
        pthread_mutex_lock(&a->lock);
    void *exists = find(a->edge_tree, b->id);
    if (exists)
    {
        topologic_debug("%s;%s;%p", "create_edge", "edge exists", (void *) NULL);
        goto exit_edge;
    }

    struct edge *edge = (struct edge *)malloc(sizeof(struct edge));
    if (!edge)
    {
        topologic_debug("%s;%s;%p", "create_edge", "failed to create edge", (void *) NULL);
        goto exit_edge;
    }

    if (a == b)
        edge->edge_type = SELF_EDGE;
    else
        edge->edge_type = EDGE;
    edge->a = a;
    edge->b = b;
    edge->bi_edge = NULL;

    edge->glbl = glbl;

    edge->a_vars = a->shared->edge_data;
    if (context != SWITCH)
        edge->b_vars = b->shared->edge_data;
    else
        edge->b_vars = NULL;

    edge->f = f;
    //MAKE UNIQUE
    edge->id = b->id;

    if (insert(a->edge_tree, edge, edge->id) < 0)
    {
        topologic_debug("%s;%s;%p", "create_edge", "failed to insert edge", (void *) NULL);
        goto free_edge;
    }

    if (context != SINGLE)
    {
        if (edge->edge_type == SELF_EDGE)
            pthread_mutex_unlock(&a->lock);
        pthread_mutex_lock(&b->lock);
    }
    if (insert(b->joining_vertices, a, a->id) < 0)
    {
        topologic_debug("%s;%s;%p", "create_edge", "failed to connect edge with b", (void *) NULL);
        goto remove_edge;
    }

    if (context != SINGLE)
    {
        pthread_mutex_unlock(&b->lock);
        if (edge->edge_type != SELF_EDGE)
            pthread_mutex_unlock(&a->lock);
    }
    topologic_debug("%s;%s;%p", "create_edge", "success", edge);
    return edge;

remove_edge:
    remove_ID(a->edge_tree, edge->id);
    if (context != SINGLE && edge->edge_type != SELF_EDGE)
        pthread_mutex_unlock(&b->lock);
free_edge:
    edge->a = NULL;
    edge->b = NULL;
    edge->a_vars = NULL;
    edge->b_vars = NULL;
    edge->f = NULL;
    edge->glbl = NULL;
    edge->id = 0;
    free(edge);
    edge = NULL;
exit_edge:
    if (context != SINGLE)
        pthread_mutex_unlock(&a->lock);
    return NULL;
}

int create_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *, void *, const void *const, const void *const), void *glbl, struct edge **edge_a_to_b, struct edge **edge_b_to_a)
{
    topologic_debug("%s;a %p;b %p;f %p;glbl %p", "create_edge", a, b, f, glbl);
    if (!a || !b || !f || a == b)
    {
        topologic_debug("%s;%s;%d", "create_bi_edge", "invalid args", -1);
        return -1;
    }
    struct edge *a_to_b, *b_to_a;
    enum CONTEXT context = a->context;

    a_to_b = create_edge(a, b, f, glbl);
    if (!a_to_b)
    {
        topologic_debug("%s;%s;%d", "create_bi_edge", "could not create edge a to b", -1);
        return -1;
    }
    a_to_b->edge_type = BI_EDGE;

    if (context != SINGLE)
    {
        if (pthread_mutex_init(&a_to_b->bi_edge_lock, NULL) < 0)
        {
            remove_edge(a, b);
            a_to_b = NULL;
            topologic_debug("%s;%s;%d", "create_bi_edge", "could not create bi_edge lock", -1);
            return -1;
        }
    }

    b_to_a = create_edge(b, a, f, glbl);
    if (!b_to_a)
    {
        remove_edge(a, b);
        free(a_to_b);
        a_to_b = NULL;
        topologic_debug("%s;%s;%d", "create_bi_edge", "could not create edge b to a", -1);
        return -1;
    }
    b_to_a->edge_type = BI_EDGE;

    b_to_a->bi_edge_lock = a_to_b->bi_edge_lock;

    a_to_b->bi_edge = b_to_a;
    b_to_a->bi_edge = a_to_b;

    if (edge_a_to_b)
        *edge_a_to_b = a_to_b;
    if (edge_b_to_a)
        *edge_b_to_a = b_to_a;
    topologic_debug("%s;%s;%d", "create_bi_edge", "success", 0);
    return 0;
}

int remove_edge(struct vertex *a, struct vertex *b)
{
    topologic_debug("%s;a %p;b %p", "remove_edge", a, b);
    if (!a || !b)
    {
        topologic_debug("%s;%s;%d", "remove_edge", "invalid args", -1);
        return -1;
    }

    remove_ID(b->joining_vertices, a->id);

    if (a->context != SINGLE)
    {
        pthread_mutex_lock(&a->lock);
    }
    void *data = remove_ID(a->edge_tree, b->id);
    if (!data)
    {
        if (a->context != SINGLE)
        {
            pthread_mutex_unlock(&a->lock);
        }
        topologic_debug("%s;%s;%d", "remove_edge", "invalid edge", -1);
        return -1;
    }

    struct edge *edge = (struct edge *)data;
    edge->a_vars = NULL;
    edge->b_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    if (edge->edge_type == BI_EDGE)
    {
        if (a->context != SINGLE)
        {
            pthread_mutex_destroy(&edge->bi_edge_lock);
        }
        edge->bi_edge->bi_edge = NULL;
        edge->bi_edge->edge_type = EDGE;
    }
    else
    {
        if (edge->glbl)
            free(edge->glbl);
    }
    edge->glbl = NULL;
    edge->bi_edge = NULL;

    free(edge);
    edge = NULL;
    if (a->context != SINGLE)
    {
        pthread_mutex_unlock(&a->lock);
    }
    topologic_debug("%s;%s;%d", "create_bi_edge", "removed", 0);
    return 0;
}

int remove_edge_id(struct vertex *a, int id)
{
    topologic_debug("%s;a %p;id %d", "remove_edge_id", a, id);
    if (!a)
    {
        topologic_debug("%s;%s;%d", "remove_edge_id", "invalid vertex", -1);
        return -1;
    }

    if (a->context != SINGLE)
    {
        pthread_mutex_lock(&a->lock);
    }
    void *data = remove_ID(a->edge_tree, id);
    if (!data)
    {
        topologic_debug("%s;%s;%d", "remove_edge_id", "invalid edge", -1);
        return -1;
    }
    struct edge *edge = (struct edge *)data;

    remove_ID(edge->b->joining_vertices, a->id);

    edge->a_vars = NULL;
    edge->b_vars = NULL;
    edge->a = NULL;
    edge->b = NULL;
    edge->f = NULL;
    edge->id = 0;

    if (edge->edge_type == BI_EDGE)
    {
        if (a->context != SINGLE)
        {
            pthread_mutex_destroy(&edge->bi_edge_lock);
        }
        edge->bi_edge->bi_edge = NULL;
        edge->bi_edge->edge_type = EDGE;
    }
    else
    {
        free(edge->glbl);
    }
    edge->glbl = NULL;
    edge->bi_edge = NULL;

    free(edge);
    edge = NULL;
    if (a->context != SINGLE)
    {
        pthread_mutex_unlock(&a->lock);
    }
    topologic_debug("%s;%s;%d", "remove_edge_id", "removed edge", 0);
    return 0;
}

int remove_bi_edge(struct vertex *a, struct vertex *b)
{
    topologic_debug("%s;a %p;b %p", "remove_bi_edge", a, b);
    if (!a || !b || a == b)
    {
        topologic_debug("%s;%s;%d", "remove_bi_edge", "invalid args", -1);
        return -1;
    }
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = remove_edge(a, b)) < 0)
        ret = -2;
    if ((b_ret = remove_edge(b, a)) < 0 && a_ret < 0)
        ret = -1;
    if (b_ret < 0 && a_ret == 0)
        ret = -3;

    topologic_debug("%s;%s;%d", "remove_bi_edge", "finished", ret);
    return ret;
}

int modify_edge(struct vertex *a, struct vertex *b, int (*f)(void *, void *, const void *const, const void *const), void *glbl)
{
    topologic_debug("%s;a %p;b %p;f %p;glbl %p", "modify_edge", a, b, f, glbl);
    if (!a || !b)
    {
        topologic_debug("%s;%s;%d", "modify_edge", "invalid args", -1);
        return -1;
    }
    if (a->context != SINGLE)
    {
        pthread_mutex_lock(&a->lock);
    }
    struct edge *edge = (struct edge *)find(a->edge_tree, b->id);
    if (!edge)
    {
        if (a->context != SINGLE)
        {
            pthread_mutex_unlock(&a->lock);
        }
        topologic_debug("%s;%s;%d", "modify_edge", "invalid edge", -1);
        return -1;
    }
    if (f)
    {
        edge->f = f;
    }
    if (glbl)
    {
        if (edge->glbl)
            free(edge->glbl);
        edge->glbl = glbl;
    }
    if (a->context != SINGLE)
    {
        pthread_mutex_unlock(&a->lock);
    }
    topologic_debug("%s;%s;%d", "modify_edge", "success", 0);
    return 0;
}

int modify_bi_edge(struct vertex *a, struct vertex *b, int (*f)(void *, void *, const void *const, const void *const), void *glbl)
{
    topologic_debug("%s;a %p;b %p;f %p;glbl %p", "modify_bi_edge", a, b, f, glbl);
    if (!a || !b || a == b)
    {
        topologic_debug("%s;%s;%d", "modify_edge", "invalid args", -1);
        return -1;
    }
    int ret = 0, a_ret = 0, b_ret = 0;
    if ((a_ret = modify_edge(a, b, f, glbl)) < 0)
        ret = -2;
    if ((b_ret = modify_edge(b, a, f, glbl)) < 0 && a_ret < 0)
        ret = -1;
    if (b_ret < 0 && a_ret == 0)
        ret = -3;
    topologic_debug("%s;%s;%d", "modif_bi_edge", "success", 0);
    return ret;
}
