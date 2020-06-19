/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

%{
#include "topologic_parser_cpp.tab.hpp"
%}

%option c++

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
"max_state"     return(MAX_STATE);
"lvl_verbose"   return(LVL_VERBOSE);
"mem_option"    return(MEM_OPT);
"context"   return(LEX_CONTEXT);
"max_loop"      return(MAX_LOOP);
[0-9]+          {yylval.val = atoi(yytext); return(VALUE);}
%%
