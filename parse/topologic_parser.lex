%{
#include "topologic_parser.tab.h"
%}

blanks          [ \t\n]+

%%
{blanks}        { /* IGNORE */ }
"{"             return(L_BRACKET);
"}"             return(R_BRACKET);
"graph"         return(GRAPH);
":"             return(COLON);
"vertices"      return(VERTICES_);
"edges"         return(EDGE_);
"bi_edges"      return(BI_EDGE_);
"["             return(L_SQUARE);
"]"             return(R_SQUARE);
","             return(COMMA);
"[0-9]+"        {yylval.val = atoi(yytext); return(VALUE);}
'\"'            return(QUOTE);
%%