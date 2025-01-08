%option noyywrap yylineno

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "bison.tab.h"
    int yyerror(const char*);
%}

%%

0 {yylval.ival = atoi(yytext); return INT_LITERAL; }
[1-9][0-9]* { yylval.ival = atoi(yytext); return INT_LITERAL; }
\+ { return PLUS; }
\- { return MINUS;}
\* { return MULT;}
\/ { return DIVIDE;}
\= { return EQUAL;}
\( { return O_PAREN;}
\) { return C_PAREN;}
[\t\n\r ] {}
[a-zA-Z] {}
. { yyerror("syntax error..."); }

%%
