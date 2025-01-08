
%union {
    int ival;
}

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #define YYDEBUG 0

    extern int yylineno;

    int checkOverflow(long, long, int);
    int yyerror(const char*);
    int yylex();
    void yyrestart(FILE *);
%}

%token PLUS MINUS DIVIDE MULT EQUAL O_PAREN C_PAREN
%token <ival> INT_LITERAL

%type <ival> STMT
%type <ival> EXPR
%type <ival> TERM
%type <ival> FACTOR

%%

STMT: EQUAL EXPR { $$ = $2; printf("Result: %d\n", $$); }
    | STMT EQUAL EXPR { printf("Result: %d\n", $3); };
EXPR: EXPR PLUS TERM { checkOverflow($1,$3,0); $$ = $1 + $3; }
    | EXPR MINUS TERM { checkOverflow($1,$3,1); $$ = $1 - $3; }
    | TERM { $$ = $1;};
TERM: TERM MULT FACTOR { checkOverflow($1,$3,2); $$ = $1 * $3; }
    | TERM DIVIDE FACTOR { checkOverflow($1,$3,3); $$ = $1 / $3; }
    | FACTOR { $$ = $1; };
FACTOR: MINUS FACTOR { $$ = (-$2); }
    | O_PAREN EXPR C_PAREN { $$ = $2; }
    | INT_LITERAL { $$ = $1; };

%%
int yydebug = 1;
int main(int argc, char *argv[])
{
    if(argc == 1)
    {
        fprintf(stderr, "no filename argument\n");
        exit(EXIT_FAILURE);
    }
   FILE * fp = fopen(argv[1], "r"); 
   
   yyrestart(fp);
   yylineno = 1;
   yyparse();
   fclose(fp);
   return EXIT_SUCCESS;
}

// note: op = 0: Addition op = 1: Subtraction op = 2: Multplication op = 3: Division
int checkOverflow(long n, long m, int op) {
    // Take two inputs try outputs to a long and to a int compare their equality. 
    // also checks divide by 0 in case 3:
    long long bit64 = 0;
    int bit32 = 0;
    switch (op) {

       case 0:
            bit64 = n + m;
            bit32 = n + m;
            if(bit64 != bit32) {
                yyerror("addition overflow");
                return 1;
            }
        break; 

        case 1:
            bit64 = n - m;
            bit32 = n - m;
            if(bit64 != bit32) {
                yyerror("subtraction overflow");
                return 1;
            }
        break;

        case 2:
            bit64 = n * m;
            bit32 = n * m;
            if(bit64 != bit32) {
                yyerror("multiplication overflow");
                return 1;
            }
        break;

        case 3:
            if(m == 0) {
                yyerror("divide by zero");
            }
            bit64 = n / m;
            bit32 = n / m;
            if(bit64 != bit32) {
                yyerror("divide overflow");
                return 1;
            }
        break;

        default:
            yyerror("fell out of checkOverflow()");
    } 
    return 0;
}

int yyerror(const char *s) {
    fprintf(stderr, "Error: %s @line %d\n", s, yylineno);
    exit(EXIT_FAILURE);
    return 0;
}