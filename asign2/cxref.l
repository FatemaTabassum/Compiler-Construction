%{
    
    /****************************************************************
     *                                                              *
     *      Name: Fatema Tabassum Liza                              *
     *      Class: COP5621                                          *
     *      Assignment: Asg 2  (Implementing a Scanner with lex)    *
     *      Compile:  "lex cxref.l"
                        "gcc -g -o cxref lex.yy.c -ll"              *
     *                                                              *
     ****************************************************************/
    
    
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 

#define LINE_STR_SIZE 20


int programLineNoCounter = 0;
int i = 0;
typedef struct Desc
    {
    char* ident;
    char* line_no_str ;
    } DescStruct;

DescStruct ident_struct_ar[1000];
DescStruct* g_ppRecords;
int total_identifier = 0;
int charLength = 0;


void insertIdentifier(char* str, int programLinecnt);
DescStruct createDescStruct( char *str, int id_line_no);
void insertIdentifier(char* str, int identifierLineNo);
void processValString(char *str);


%}

%%
auto                    ;
break                   ;
case                    ;
char                    ;
continue                ;
default                 ;
do                      ;
double                  ;
else                    ;
extern                  ;
float                   ;
for                     ;
goto                    ;
if                      ;
int                     ;
long                    ;
register                ;
return                  ;
short                   ;
sizeof                  ;
static                  ;
struct                  ;
switch                  ;
typedef                 ;
union                   ;
unsigned                ;
void                    ;
while                   ;


#.*                     ;
^[[:blank:]]#.* { ; }   ;
'[^'\n]*'               ;
\"(\\.|[^"\\])*\"               ;
"//".*|"/*"(.*[\n].*)*"*/"      ;

[a-zA-Z_][a-zA-Z0-9_]*   {
                            insertIdentifier(yytext, programLineNoCounter + 1);
                        }
"\n"                    {
                            programLineNoCounter++;
                        }

.                   ;

%%


void processValString(char *str) {
    const char *s = ",";
    char *token = strtok(str, ",");
    int *all_val;
    all_val = (int *)malloc(sizeof(int) * ((strlen(str)/2) + 10));
    int idx = 0;
    while (token != NULL) {
        all_val[idx] = atoi(token);
        idx++;
        token = strtok(NULL, s);
    }
    int i, j;

    for (i = 0; i < idx - 1; ++i) {
        for (j = i + 1; j < idx; ++j) {
            if (all_val[i] == all_val[j]) {
                all_val[j] = -1;
            }
        }
    }
    for (i = 0; i < idx; ++i) {
        if (all_val[i] > 0) {
            if(i < idx - 1){
            printf(" %d,", all_val[i]);
            }
            else{
            printf(" %d", all_val[i]);
            }
        }
    }
    printf("\n");
    return ;
}

void insertIdentifier(char* str, int programLinecnt) {

    DescStruct idDesc = createDescStruct(str, programLinecnt);
    
    char num[23];
    sprintf ( num, "%d", programLinecnt);
    static char string[100];
    memset(string, 0, 100);
    
    int iter;
    bool flag = false;
    for(iter = 0; iter < total_identifier; iter++)
    {
        if (strcmp(idDesc.ident, g_ppRecords[iter].ident) == 0 )
        {
            strcat( string, "," );
            strcat( string, num );
            g_ppRecords[iter].line_no_str = (char *)realloc(g_ppRecords[iter].line_no_str , strlen(g_ppRecords[iter].line_no_str) + strlen(num) + 10);
            strcat ( g_ppRecords[iter].line_no_str, string );
            strcat ( g_ppRecords[iter].line_no_str, "\0" );
            flag = true;
            break;;
        }
    }
    if (!flag) {
        
        strcpy(idDesc.line_no_str, num);
        g_ppRecords = (DescStruct *)realloc(g_ppRecords,sizeof(DescStruct) * (total_identifier + 3));
        g_ppRecords[total_identifier]  = idDesc;
        total_identifier++;
    }
    
}


 DescStruct createDescStruct( char *str, int id_line_no) {
     DescStruct idDesc;
     idDesc.ident = malloc ( ( strlen(str) + 1) * sizeof(char) );
     strcpy(idDesc.ident, str);
     idDesc.line_no_str = (char *) malloc ( LINE_STR_SIZE * sizeof(char) + 10);
     return idDesc;
    
}


int cmpShift(const void* lhs, const void* rhs)
{
    return strcmp(((DescStruct*)lhs)->ident, ((DescStruct*)rhs)->ident);
}



int main() {
    g_ppRecords =  (DescStruct *) malloc( 1 * (sizeof(DescStruct)));
    yylex();
    int i,j;
    qsort(g_ppRecords, total_identifier, sizeof(DescStruct), cmpShift);
    for ( i = 0; i < total_identifier ; i++) {
        printf("%25s:",g_ppRecords[i].ident);
        processValString(g_ppRecords[i].line_no_str);
    }
}

