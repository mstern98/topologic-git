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
%token MAX_STATE
%token LVL_VERBOSE
%token CONTEXT
%token MEM_OPT
%token <val> VALUE

%start json
%%
json: L_BRACKET GRAPH   {*graph = GRAPH_INIT(); if (!(*graph)){fprintf(stderr, "Can't create graph\n"); return -1;}}  
      COLON L_BRACKET content R_BRACKET
      R_BRACKET
      ;
content: /* empty */{printf(" NIL \n");}
        | params g
        | g
        | params
        ;
params: verb COMMA params
        | state COMMA params
        | mem_opt COMMA params
        | context COMMA params
        | verb
        | mem_opt
        | context
        | state
        |
        ;
state: MAX_STATE COLON VALUE {if ($3 < 0) {fprintf(stderr, "Invalid Max State Changes %d\n", $3); return -1;} (*graph)->max_state_changes = $3;}
     ;
verb: LVL_VERBOSE COLON VALUE {(*graph)->lvl_verbose = $3;}
    ;
context: CONTEXT COLON VALUE {(*graph)->context = $3;}
       ;
mem_opt: MEM_OPT COLON VALUE {(*graph)->mem_option = $3;}
       ;
g:  vs COMMA es COMMA bes
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