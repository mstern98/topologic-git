// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"

void print_edges(struct graph *graph, struct AVLTree *edges, const char *indent, FILE *out)
{
    topologic_debug("%s;%p", "print_edges", graph);
    struct stack *stack = init_stack();
    struct edge *edge = NULL;
    preorder(edges, stack);
    fprintf(out, "%s\"Edges\": {\n", indent);
    while ((edge = (struct edge *)pop(stack)) != NULL)
    {
        fprintf(out, "%s\t\"%d\": {\n", indent, edge->id);
        fprintf(out, "%s\t\t\"id\": %d,\n", indent, edge->id);
        fprintf(out, "%s\t\t\"from\": \"%p\",\n", indent, edge->a);
        fprintf(out, "%s\t\t\"to\": \"%p\",\n", indent, edge->b);
        fprintf(out, "%s\t\t\"edge_type\": %d,\n", indent, edge->edge_type);
        fprintf(out, "%s\t\t\"bi_edge\": \"%p\"", indent, edge->bi_edge);
        if ((graph->lvl_verbose & FUNCTIONS) == FUNCTIONS)
        {
            fprintf(out, ",\n%s\t\t\"f\": \"%p\"", indent, edge->f);
        }
        if ((graph->lvl_verbose & GLOBALS) == GLOBALS)
        {
            fprintf(out, ",\n%s\t\t\"glbl\": \"%p\"", indent, edge->glbl);
        }
        fprintf(out, "\n%s\t}", indent);
        if (stack->length > 0)
            fprintf(out, ",\n");
        else
            fprintf(out, "\n");
    }
    fprintf(out, "%s}", indent);
    destroy_stack(stack);
    topologic_debug("%s;%s", "print_edges", "finished");
}

void print_state(struct graph *graph, FILE *out)
{
    topologic_debug("%s;%p", "print_state", graph);
    /*Called by print and does a pre-order traversal of all the data in each vertex*/
    int vertex_id = 0;
    void *glbl = NULL;
    union shared_edge *edge_shared = NULL;
    void (*f)(int, struct graph *, struct vertex_result *, void *, void *) = NULL;
    struct vertex *v = NULL;
    struct stack *stack = init_stack();

    inorder(graph->vertices, stack);
    fprintf(out, "\t\"Vertices\": {\n");
    while ((v = (struct vertex *)pop(stack)) != NULL)
    {
        vertex_id = v->id;
        f = v->f;
        edge_shared = v->shared;
        glbl = v->glbl;

        if ((graph->lvl_verbose & VERTICES) == VERTICES)
        {
            fprintf(out, "\t\t\"%d\": {\n", vertex_id);
            fprintf(out, "\t\t\t\"id\": %d,\n", vertex_id);
            fprintf(out, "\t\t\t\"active\": %d", v->is_active);
            if ((graph->lvl_verbose & FUNCTIONS) == FUNCTIONS)
            {
                fprintf(out, ",\n\t\t\t\"f\": \"%p\"", f);
            }
            if ((graph->lvl_verbose & GLOBALS) == GLOBALS)
            {
                fprintf(out, ",\n\t\t\t\"glbl\": \"%p\",\n", glbl);
                fprintf(out, "\t\t\t\"edge_shared\": \"%p\"", edge_shared);
            }
            fprintf(out, ",\n\t\t\t\"edges:\": %d", v->edge_tree->size);
            if ((graph->lvl_verbose & EDGES) == EDGES)
            {
                fprintf(out, ",\n");
                print_edges(graph, v->edge_tree, "\t\t\t", out);
            }
            fprintf(out, "\n\t\t}");
        }
        else if ((graph->lvl_verbose & EDGES) == EDGES)
        {
            fprintf(out, "\t\t\"%d\": {\n", vertex_id);
            print_edges(graph, v->edge_tree, "\t\t\t", out);
            fprintf(out, "\n\t\t}");
        }
        if (stack->length > 0)
            fprintf(out, ",\n");
        else
            fprintf(out, "\n");
    }
    fprintf(out, "\t}\n");
    destroy_stack(stack);
    topologic_debug("%s;%s", "print_state", "finished");
}

void print_graph(struct graph *graph)
{
    topologic_debug("%s;%p", "print_graph", graph);
    if (!graph)
    {
        topologic_debug("%s;%s", "print_graph", "invalid graph");
        return;
    }
    if (graph->context != SINGLE)
        pthread_mutex_lock(&graph->lock);
    if (graph->lvl_verbose == NO_VERB || graph->snapshot_timestamp == -1)
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        topologic_debug("%s;%s", "print_graph", "no print");
        return;
    }

    if ((graph->snapshot_timestamp == START_STOP && (graph->num_vertices != 0 ||
                                                     (graph->state_count != 0 && graph->state_count != graph->max_state_changes - 1))) ||
        (graph->snapshot_timestamp != 0 && graph->state_count % graph->snapshot_timestamp != 0))
    {
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        topologic_debug("%s;%s", "print_graph", "no print");
        return;
    }

    char buffer[256];
    sprintf(buffer, "state_%d.json", graph->state_count);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    FILE *out = fopen(buffer, "w+");
    if (!out)
    {
        topologic_debug("%s;%s", "print_graph", "Failed to open file");
        return;
    }
#else
    int dirfd = open("./", O_DIRECTORY | O_RDONLY);
    if (dirfd == -1)
    {
        topologic_debug("%s;%s", "print_graph", "Failed to open file");
        return;
    }
    int fd = openat(dirfd, buffer, O_TRUNC | O_CREAT | O_WRONLY, S_IRWXU);
    if (fd == -1)
    {
        close(dirfd);
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        topologic_debug("%s;%s", "print_graph", "Failed to open file");
        return;
    }
    FILE *out = fdopen(fd, "w");
    if (!out)
    {
        perror("");
        topologic_debug("%s;%s", "print_graph", "Failed to open file");
        close(fd);
        close(dirfd);
        if (graph->context != SINGLE)
            pthread_mutex_unlock(&graph->lock);
        return;
    }
#endif

    /**TODO: Print enums**/
    fprintf(out, "{\n");
    fprintf(out, " \"graph\": {\n");
    fprintf(out, "\t\"state\": %d,\n\t\"max_state\": %d,\n\t\"max_loop\": %d,\n\t\"timestamps\": %d,\n\t\"verbosity\": %d,\n\t\"vertices\": %d,\n", graph->state_count,
            graph->max_state_changes,
            graph->max_loop,
            graph->snapshot_timestamp,
            graph->lvl_verbose,
            graph->vertices->size);
    print_state(graph, out);
    fprintf(out, " }\n");
    fprintf(out, "}\n");

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    fclose(out);
#else
    fclose(out);
    close(dirfd);
#endif
    if (graph->context != SINGLE)
        pthread_mutex_unlock(&graph->lock);
    topologic_debug("%s;%s", "print_graph", "finished");
}
