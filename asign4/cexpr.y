%{
    
    
    /****************************************************************
     *                                                              *
     *      Name: Fatema Tabassum Liza                              *
     *      Class: COP5621                                          *
     *      Assignment: Asg 4 (Implementing a Calculator)           *
     *                                                              *
     ****************************************************************/
    
    
    
    /********* C declarations used in actions *****/
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <stdbool.h>
    
    #define ARRAY_SIZE 30
    #define SYMBOLS_SIZE 26
    
    
    /********** struct **********/
    
    typedef struct symbolVal_{
        bool is_known;
        int val;
    }symbolValStruct;
    
    
    /****** global variables *********/
    
    symbolValStruct symbolsStructArray[ARRAY_SIZE];
    int getSymbolVal(char symbol);
    void updateSymbolVal(char symbol, int val);
    int val; // will contain the value of equal operator;
    char varPlusPlus;
    char *unknown_str = "unknown\0";
    bool expression_is_known = true;
    
    /******* function prototype *******/
    
    void yyerror (char *s);
    int yylex();
    void updateSymbolVal(char symbol, int val);
    int getSymbolVal(char symbol);
    int computeSymbolIndex(char token);
    void setInitialValues(void);
    bool checkIfSymbolValid(char symbol);
    void dump(void);
    void reset(void);
    
%}

%union {
    int num; char id;
}

%start line


%token DUMP
%token RESET
%token PLUSPLUS
%token MINUSMINUS
%token LSHIFT
%token RSHIFT

%token <num> number
%token <id> identifier

%type <num> line expr expr0 expr1 expr2 expr3 expr4 expr5 expr6 expr7 expr8 factor

%%


line    : expr ';'                      {
    if(!expression_is_known) {
        printf("unknown\n");
        expression_is_known = true;
    } else {
        printf("%d\n", $1);
    }
}
| line expr ';'                 {
    if(!expression_is_known) {
        printf("unknown\n");
        expression_is_known = true;
    }else {
        printf("%d\n", $2);
    }
}
| line DUMP                      { dump(); }
| line RESET                     { reset(); }
| DUMP                           { dump(); }
| RESET                          { reset(); }
;


expr    : identifier '=' expr   {
    if(expression_is_known) {
        updateSymbolVal($1,$3);
        $$ = getSymbolVal($1);
    } else {
        symbolsStructArray[computeSymbolIndex($1)].is_known = false;
    }
}
|expr0                              {$$ = $1;}
;



expr0    :   expr1               { $$ = $1; }
|   expr0 '|' expr1              { $$ = $1 | $3; }
;



expr1   :   expr2               { $$ = $1; }
|   expr1 '^' expr2             { $$ = $1 ^ $3; }
;


expr2    :   expr3              { $$ = $1; }
|   expr2 '&' expr3             { $$ = $1 & $3; }
;



expr3    :   expr4              { $$ = $1; }
|   expr3 LSHIFT expr4          { $$ = $1 << $3; }
|   expr3 RSHIFT expr4          { $$ = $1 >> $3; }
;




expr4     : expr4 '+' expr5      { $$ = $1 + $3;
}
| expr4 '-' expr5               {   $$ = $1 - $3; }
| expr5                         { $$ = $1; }
;




expr5    : expr5 '*' expr6      { $$ = $1 * $3; }
| expr5 '/' expr6               { $$ = $1 / $3; }
| expr5 '%' expr6               { $$ = $1 % $3; }
| expr6                         { $$ = $1;}
;


expr6    :   expr7          { $$ = $1;}
|   '~'  expr6              { $$ = ~$2; }
|   '-'  expr6              { $$ = -$2; }
;



expr7    : identifier PLUSPLUS           {
    if(checkIfSymbolValid($1)) {
        int v;
        v = getSymbolVal($1);
        $$ = v;
        updateSymbolVal($1,v + 1);
        
    } else {
        expression_is_known = false;
        ;
    }
}
| identifier MINUSMINUS                  {
    if(checkIfSymbolValid($1)) {
        int v;
        v = getSymbolVal($1);
        $$ = v;
        updateSymbolVal($1,v - 1);
    } else {
        expression_is_known = false;
        ;
    }
}
| expr8                             {$$ = $1;}
;



expr8    : PLUSPLUS identifier       {
    if(checkIfSymbolValid($2)) {
        int v;
        v = getSymbolVal($2);
        $$ = v + 1;
        updateSymbolVal($2,v + 1);
    } else {
        expression_is_known = false;
    }
    
}

|  MINUSMINUS identifier             {
    if(checkIfSymbolValid($2)) {
        int v;
        v = getSymbolVal($2);
        $$ = v - 1;
        updateSymbolVal($2,v - 1);
    } else {
        expression_is_known = false;
    }
    
}

| factor                        {$$ = $1;}
;




factor  : number            {$$ = $1;}
| identifier                {
    if(checkIfSymbolValid($1) == false) {
        expression_is_known = false;
    } else {
        expression_is_known = true;
        $$ = getSymbolVal($1);
        varPlusPlus = $1;
    }
}
| '(' expr ')'              { $$ = $2; }
;

%%

/************* C code ***********/


main()
{
    if (yyparse())
    printf("\nInvalid expression.\n");
    else
    printf("\nCalculator off.\n");
}



/****** returns the the index of a given token *****/
int computeSymbolIndex(char token) {
    return token - 'a';
    
}

/****** returns the value of a given symbol *****/

bool checkIfSymbolValid(char symbol) {
    int bucket = computeSymbolIndex(symbol);
    return symbolsStructArray[bucket].is_known;
}


/****** returns the value of a given symbol *****/

int getSymbolVal(char symbol) {
    int bucket = computeSymbolIndex(symbol);
    return symbolsStructArray[bucket].val;
}

/*****  updates the value of a given symbol *****/

void updateSymbolVal(char symbol, int val) {
    int bucket = computeSymbolIndex(symbol);
    
    symbolsStructArray[bucket].is_known = true;
    symbolsStructArray[bucket].val = val;
}

/***** init symbol table *****/

void setInitialValues() {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        symbolsStructArray[i].is_known = false;
        symbolsStructArray[i].val = 0;
    }
}

void dump() {
    int i;
    for (i = 0; i < SYMBOLS_SIZE; i++) {
        if (!symbolsStructArray[i].is_known) {
            printf("%c: unknown\n",i + 'a');
        } else {
            printf("%c: %d\n", i + 'a', symbolsStructArray[i].val);
        }
    }
}

void reset(void) {
    setInitialValues();
}

void yyerror (char *s) {fprintf (stderr, "%s\n", s);}

