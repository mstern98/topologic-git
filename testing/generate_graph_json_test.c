// SPDX-License-Identifier: MIT WITH bison-exception
// Copyright © 2020 Matthew Stern, Benjamin Michalowicz

#include "../include/topologic.h"
#include <assert.h>

int main() {
    fprintf(stderr, "PARSE JSON\n");
    FILE *f = fopen("./testing/graph_test.json", "r");
    if (!f) {
        fprintf(stderr, "FILE NOT FOUND\n");
        return -1;
    }
    struct graph *graph = parse_json(f);
    fclose(f);

    assert(graph != NULL);
    struct vertex *v = NULL;
    assert((v = (struct vertex *) find(graph->vertices, 1)) != NULL);
    struct edge *e = NULL;
    assert((e = (struct edge *) find(v->edge_tree, 4)) != NULL);
    assert(e->edge_type == BI_EDGE);

    print(graph);
    destroy_graph(graph);
    graph = NULL;
    assert(graph == NULL);
    fprintf(stderr, "JSON PARSING WORKS\n");
    return 0;
}