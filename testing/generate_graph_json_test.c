#include "../include/topologic.h"
#include <assert.h>

int main() {
    FILE *f = fopen("./testing/graph_test.json", "r");
    if (!f) {
        fprintf(stderr, "FILE NOT FOUND\n");
        return -1;
    }
    struct graph *graph = parse_json(f);
    fclose(f);

    assert(graph != NULL);

    destroy_graph(graph);
    graph = NULL;
    return 0;
}