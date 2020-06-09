%{
#include "../include/topologic.h"
struct vertex_result *f(void * args) {return NULL;}
int yylex();
void yyerror(struct graph** graph, const char *s);
extern FILE *yyin;
%}

%union {
    struct graph *graph;
    int val;
};

%parse-param {struct graph** graph}
%token L_BRACKET 
%token R_BRACKET
%token GRAPH
%token COLON
%token VERTICES_
%token EDGE_
%token BI_EDGE_
%token L_SQUARE
%token R_SQUARE
%token COMMA
%token QUOTE
%token <val> VALUE

%start json
%%
json: L_BRACKET GRAPH g R_BRACKET {*graph = GRAPH_INIT(); fprintf(stderr, "created GRAPH: %p\n", *graph);}
      ;
g:  /* empty */ {fprintf(stderr, " IN G\n");}
    | vs COMMA es COMMA bes
    | vs COMMA es
    | vs COMMA bes
    | vs
    ;
vs: VERTICES_ COLON L_SQUARE v R_SQUARE
    ;
v:  /* empty */
    | VALUE COMMA v {create_vertex(*graph, f, $1, NULL);}
    | VALUE {create_vertex(*graph, f, $1, NULL);}
    ;
es: EDGE_ COLON L_BRACKET e R_BRACKET
    ;
e:  /* empty */
    | VALUE COLON VALUE COMMA e {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (!a && !b) create_edge(a, b, NULL, NULL);}
    | VALUE COLON VALUE {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (!a && !b) create_edge(a, b, NULL, NULL);}
    ;
bes:BI_EDGE_ COLON L_BRACKET be R_BRACKET
    ;
be: /* empty */
    | VALUE COLON VALUE COMMA be {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (!a && !b) create_edge(a, b, NULL, NULL);}
    | VALUE COLON VALUE {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (!a && !b) create_edge(a, b, NULL, NULL);}
    ;
%%

void yyerror(struct graph** graph, const char *s) {
    fprintf(stderr, "yerror: %s\n", s);
    destroy_graph(*graph);
    *graph = NULL;
}

struct graph *parse_json(FILE *file) {
    if (!file) return NULL;

    fprintf(stderr, "PARSE JSON\n");
    yyin = file;
    struct graph *graph = NULL;
    yyparse(&graph);
    yyin = NULL;
    return graph;
}