%{
    
    
    /****************************************************************
     *                                                              *
     *      Name: Fatema Tabassum Liza                              *
     *      Class: COP5621                                          *
     *      Assignment: Asg 4 (Implementing a Calculator)           *
     *                                                              *
     ****************************************************************/
    
    
    
    #include "y.tab.h"
    void yyerror (char *s);
    int yylex();
    
    
%}

%%

"reset"                     {return RESET;}
"dump"                      {return DUMP;}
[a-z]                       {yylval.id = yytext[0]; return identifier;}
[0-9]+                      {yylval.num = atoi(yytext); return number;}

"++"                        { return PLUSPLUS; }
"--"                        { return MINUSMINUS; }
"<<"                        { return LSHIFT; }
">>"                        { return RSHIFT; }

[-+=/\*%&]                  {return yytext[0];}
[\^~\(\);\|]                {return yytext[0];}

[ \t\n]+                        ;
.                               ;




%%
int yywrap (void) {
    
    return 1;
}


