%{
#include "../include/topologic.h"
struct vertex_result *f(void *args) {return NULL;}
int edge_f(void *args) {return 0;}
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
%token <val> VALUE

%start json
%%
json: L_BRACKET GRAPH   {*graph = GRAPH_INIT();}  
      COLON L_BRACKET g R_BRACKET 
      R_BRACKET 
      ;
g:  /* empty */
    | vs COMMA es COMMA bes
    | vs COMMA bes COMMA es
    | vs COMMA es
    | vs COMMA bes
    | vs
    ;
vs: VERTICES_ COLON L_SQUARE v R_SQUARE
    ;
v:  /* empty */
    | VALUE COMMA {create_vertex(*graph, f, $1, NULL);}
    v
    | VALUE {create_vertex(*graph, f, $1, NULL);}
    ;
es: EDGE_ COLON L_BRACKET e R_BRACKET
    ;
e:  /* empty */
    | VALUE COLON VALUE COMMA {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (a && b) create_edge(a, b, edge_f, NULL);}
      e
    | VALUE COLON VALUE {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (a && b) create_edge(a, b, edge_f, NULL);}
    ;
bes:BI_EDGE_ COLON L_BRACKET be R_BRACKET
    ;
be: /* empty */
    | VALUE COLON VALUE COMMA {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (a && b) create_bi_edge(a, b, edge_f, NULL, NULL, NULL);}
      be
    | VALUE COLON VALUE {struct vertex *a = find((*graph)->vertices, $1); struct vertex *b = find((*graph)->vertices, $3); if (a && b) create_bi_edge(a, b, edge_f, NULL, NULL, NULL);}
    ;
%%

void yyerror(struct graph** graph, const char *s) {
    fprintf(stderr, "yerror: %s\n", s);
    destroy_graph(*graph);
    *graph = NULL;
}

struct graph *parse_json(FILE *file) {
    if (!file) return NULL;
    yyin = file;
    struct graph *graph = NULL;
    yyparse(&graph);
    yyin = NULL;
    return graph;
}