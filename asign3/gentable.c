/****************************************************************
 *                                                              *
 *      Name: Fatema Tabassum Liza                              *
 *      Class: COP5621                                          *
 *      Assignment: Asg 3 (Implementing a C Parser)            *
 *      Compile: "gcc -std=c99 gentable.c"                      *
 *                                                              *
 ****************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>


/***************** Macros ***************/
#define AUGMENTED_GRAMMAR "Augmented Grammar\0"
#define FOLLOWS "Follows\0"
#define SETS_OF_LR_ITEMS "Sets of LR(0) Items\0"
#define GOTO "goto\0"
#define START_SYMBOL "'\0"
#define INITIAL_SIZE 110
#define EXTRA_SPACE 10
#define TERM_NON_TERM_SIZE 53
#define STRING_SIZE 20

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*************** Enums and Structs *****************/
enum input {
    augmented_grammar,
    follow,
    sets_of_LR_items,
} input_type;

typedef struct follow_ {
    char* non_terminal;
    int non_terminal_idx;
    char* terminal_str;
} follow_struct;

typedef struct productions_ {
    char *prod_lhs;
    char *prod_rhs;
    char *nonterminal_str;
    char *terminal_str;
    char *production_itself;
} productions_struct;


typedef struct items_ {
    int from_state;
    int to_state;
    int transition_str_pos;
    int reduce_lhs;
    bool shift;
    bool reduce;
    bool goto_;
    bool accept;
    int reduce_production_no;
    int final_item;
    
} items_struct;



/****************** Global Variables **********/
FILE *file;
FILE *in_file;
char* str;
int tot_number_of_follows;
int tot_number_of_productions;
int tot_number_of_states;
int tot_number_of_terminals;
int tot_number_of_non_terminals;
int current_state_no = -1;
int item_list_current_pos = 0;

follow_struct * follow_lists;
productions_struct *production_lists;
items_struct *item_lists;

char non_terminals_list[TERM_NON_TERM_SIZE][STRING_SIZE];
char terminals_list[TERM_NON_TERM_SIZE][STRING_SIZE];


static char** action;
static int** action_num;
static int** go_to;
static int* reduce_lhs;
static int* reduce_num;

/**************** Function Prototypes **********/

void take_and_process_input(void);
void allocate_necessary_memory(void);
void reset_globals_variables(void);
bool check_follow_starting(char *str);
bool check_if_a_production( char * str);
bool check_if_a_state(char *str);
void insert_into_follow_list(char *str);
void process_LR_item(char *str);
void print_to_stdout(void);
follow_struct allocate_mem_for_follow_struct(void);
productions_struct allocate_mem_for_production_struct(void);
void allocate_production_lists(void);
void allocate_follow_lists(void);
void allocate_item_lists(void);
void insert_into_production_list(char *str);
void insert_into_LR_items_list(char *str);
void build_table(void);
void build_reduce_arrays(void);
void allocate_and_initialize_print_array(void);
void print_define(void);
void print_action(void);
void print_action_num(void);
void print_go_to(void);
void print_reduce_num(void);
void print_reduce_lhs(void);
void print_tokens(void);

/************** main ***************/

int main(int argc, const char * argv[]) {
    
    allocate_necessary_memory();
    reset_globals_variables();
    take_and_process_input();
    print_to_stdout();
}

/************ take_and_process_input ***************/

void take_and_process_input() {
    
    char *str_ = malloc(INITIAL_SIZE * sizeof(char));
    while (fgets(str_, INITIAL_SIZE, stdin)) {
        
        str = strtok(str_, "\n");
        if (str == NULL) {
            continue;
        }
        if (!strcmp(AUGMENTED_GRAMMAR, str)) {
            input_type = augmented_grammar;
        } else if (!strcmp(FOLLOWS, str)) {
            input_type = follow;
        } else if(!strcmp(SETS_OF_LR_ITEMS, str)){
            input_type = sets_of_LR_items;
        }
        
        switch (input_type) {
            case augmented_grammar:
                if(check_if_a_production(str)) {
                    insert_into_production_list(str);
                    tot_number_of_productions++;
                }
                break;
            case follow: {
                if(check_follow_starting(str)) {
                    insert_into_follow_list(str);
                }
            }
                break;
            case sets_of_LR_items:
                if(check_if_a_state(str)) {
                    str = strtok(str, "I");
                    str = strtok(str, ":");
                    current_state_no = atoi(str);
                } else if(check_if_a_production(str)) {
                    
                    char * pch;
                    pch = strstr (str, GOTO);
                    if (pch) {
                        
                        insert_into_LR_items_list(str);
                    } else {
                        process_LR_item(str);
                    }
                }
                break;
            default:
                break;
        }
    }
}


/************ insert into production ***************/

void insert_into_production_list(char *str) {
    
    strcpy(production_lists[tot_number_of_productions].production_itself,str );
    
    const char *s = ">";
    char *token = strtok(str, s);
    token[strlen(token) - 1] = '\0';
    
    strcpy(production_lists[tot_number_of_productions].prod_lhs,token );
    token = strtok(NULL, s);
    strcpy(production_lists[tot_number_of_productions].prod_rhs,token );
    char *term;
    char *non_term;
    int term_idx = 0;
    int non_term_idx = 0;
    
    non_term = malloc(102 * sizeof(char));
    term = malloc(102 * sizeof(char));
    for (int i = 0;  i < strlen(token); i++) {
        if (token[i] >= 'A' && token[i] <= 'Z') {
            non_term[non_term_idx++] = token[i];
            non_term[non_term_idx++] = ',';
        } else {
            term[term_idx++] = token[i];
            term[term_idx++] = ',';
        }
    }
    non_term[non_term_idx] = '\0';
    term[term_idx] = '\0';
    strcpy(production_lists[tot_number_of_productions].nonterminal_str, non_term);
    strcpy(production_lists[tot_number_of_productions].terminal_str, term);
    bool flag_term_cnt = true;
    bool flag_non_term_cnt = true;
    if (strlen(term) == 0) {
        flag_term_cnt = false;
    }
    if (strlen(non_term) == 0) {
        flag_non_term_cnt = false;
    }
    /********** identify terminals ***********/
    
    token = NULL;
    token = strtok(term, ",");
    while (token != NULL) {
        
        flag_term_cnt = true;
        
        for (int j = 0; j < TERM_NON_TERM_SIZE; j++) {
            if (strcmp(terminals_list[j], token) == 0) {
                flag_term_cnt = false;
                break;
            }
        }
        if (flag_term_cnt) {
            strcpy(terminals_list[tot_number_of_terminals++], token);
        }
        token = strtok(NULL, ",");
    }
    /********** identify non_terminals ***********/
    token = NULL;
    token = strtok(non_term, ",");
    while (token != NULL) {
        flag_non_term_cnt = true;
        for (int j = 0; j < TERM_NON_TERM_SIZE; j++) {
            if (strcmp(non_terminals_list[j], token) == 0) {
                flag_non_term_cnt = false;
                break;
            }
        }
        if (flag_non_term_cnt) {
            strcpy(non_terminals_list[tot_number_of_non_terminals++], token);
        }
        token = strtok(NULL, ",");
    }
    
}

/************ insert into follow ***************/

void insert_into_follow_list(char *str) {
    const char *s = " ";
    char *token = strtok(str, s);
    strcpy(follow_lists[tot_number_of_follows].non_terminal, token);
    int idx = 0;
    while (true) {
        idx++;
        token = strtok(NULL, s);
        if(token == NULL) {
            break;
        }
        
        for (int i = 0; i < strlen(token); i++) {
            
        }
        
        strcpy(follow_lists[tot_number_of_follows].terminal_str, token);
        for (int i = 0; i < tot_number_of_terminals; i++) {
            if (!strcmp(terminals_list[i], token)) {
                follow_lists[tot_number_of_follows].non_terminal_idx = i;
            }
        }
    }
    tot_number_of_follows++;
}

/************ return an item ***************/

items_struct get_an_item() {
    items_struct itm;
    itm.from_state = -1;
    itm.to_state = -1;
    itm.goto_ = false;
    itm.shift = false;
    itm.reduce = false;
    itm.accept = false;
    itm.final_item = false;
    itm.reduce_lhs = -1;
    itm.reduce_production_no = -1;
    itm.transition_str_pos = -1;
    return itm;
}

/************ insert into LR_items_list ***************/

void insert_into_LR_items_list(char *str) {
    items_struct itm = get_an_item();
    itm.from_state = current_state_no;
    char * token;
    char * str1;
    char * str2;
    token = strtok(str, "=\0");
    str1 = token;
    str2 = strtok(NULL, "=\0");
    char subbuff[51];
    char * pch;
    pch = strstr (str, GOTO);
    int position = pch - str1;
    
    for (int i = position; i < strlen(str1) ; i++) {
        if (str1[i] == '(') {
            int len = strlen(str1) - i - 2;
            strncpy(subbuff, &str1[i+1], 1);
            subbuff[len] = '\0';
            break;
        }
    }
    
    for (int i = 0; i < TERM_NON_TERM_SIZE; i++) {
        if (pch && strcmp(subbuff, non_terminals_list[i]) == 0) {
            itm.goto_ = true;
        } else if (pch && strcmp(subbuff, terminals_list[i]) == 0) {
            itm.shift = true;
        }
    }
    
    if (itm.goto_) {
        for (int i = 0; i < tot_number_of_non_terminals; i++) {
            if (strcmp(non_terminals_list[i], subbuff) == 0) {
                itm.transition_str_pos = i;
                break;
            }
        }
    } else if (itm.shift) {
        for (int i = 0; i < tot_number_of_terminals; i++) {
            if (strcmp(terminals_list[i], subbuff) == 0) {
                itm.transition_str_pos = i;
                break;
            }
        }
    }
    
    token = NULL;
    token = strtok(str2, "I");
    itm.to_state = atoi(token);
    
    item_lists[item_list_current_pos++] = itm;
    tot_number_of_states = MAX(current_state_no, tot_number_of_states);
    tot_number_of_states = MAX(itm.to_state, tot_number_of_states);
}

/*********** parse/process_LR_item  ************/

void process_LR_item(char *str) {
    items_struct itm = get_an_item();
    char *token = malloc( INITIAL_SIZE * sizeof(char));
    char *token1 = strtok(str, " ");
    strcpy(token, token1);
    char *tok = strtok(token1, ">");
    tok[strlen(tok)-1] = '\0';
    for (int i = 0; i < strlen(token); i++) {
        if ((token[i] == '@' && i == strlen(token) - 1) &&
            (strcmp(START_SYMBOL, tok) == 0)) {
            itm.accept = true;
            token[i] = '\0';
        } else if (token[i] == '@' && i == strlen(token) - 1) {
            itm.reduce = true;
            token[i] = '\0';
        }
    }
    if (itm.accept) {
        itm.from_state = current_state_no;
        itm.transition_str_pos = tot_number_of_terminals;
        item_lists[item_list_current_pos++] = itm;

    } else if (itm.reduce) {
        itm.from_state = current_state_no;
        
        if (!strcmp(production_lists[0].production_itself, token)) {
            itm.final_item = true;
            item_lists[item_list_current_pos++] = itm;
            return;
        }
        
        for (int i = 0; i < tot_number_of_productions; i++) {
            if (!strcmp(production_lists[i].production_itself, token)) {
                itm.reduce_production_no = i;
                break;
            }
        }
        
        token = strtok(token, ">");
        token[strlen(token) - 1] = '\0';
        
        for (int i = 0; i < tot_number_of_non_terminals; i++) {
            if (!strcmp(non_terminals_list[i], token)) {
                itm.reduce_lhs = i;
            }
        }
        item_lists[item_list_current_pos++] = itm;
    }
}


/************** Memory Allocation ************/

void allocate_necessary_memory() {
    
    allocate_follow_lists();
    allocate_production_lists();
    allocate_item_lists();
}


/************ allocate Follow list ************/

void allocate_follow_lists() {
    
    follow_lists = malloc( INITIAL_SIZE * sizeof(follow_struct) );
    if (follow_lists == NULL) {
        fprintf(stderr,"%s", "Memory Allocation failed follow_lists\n");
    }
    for (int i = 0; i < INITIAL_SIZE; i++) {
        
        follow_lists[i] = allocate_mem_for_follow_struct();
    }
}

/************ allocate Follow struct ************/

follow_struct allocate_mem_for_follow_struct() {
    follow_struct f;
    f.non_terminal = malloc( INITIAL_SIZE * sizeof(char) );
    f.terminal_str = malloc( INITIAL_SIZE * sizeof(char) );
    f.non_terminal_idx = -1;
    
    if (f.non_terminal== NULL || f.terminal_str == NULL) {
        fprintf(stderr, "%s","Memory Allocation failed follow_struct\n");
    }
    return f;
}

/***** Allocate Productions List *****/

void allocate_production_lists() {
    production_lists = malloc( INITIAL_SIZE * sizeof(productions_struct) );
    if (production_lists == NULL) {
        fprintf(stderr,"%s","Memory Allocation failed production_lists\n");
    }
    for (int i = 0; i < INITIAL_SIZE; i++) {
        
        production_lists[i] = allocate_mem_for_production_struct();
    }
}

/***** Allocate Productions struct *****/

productions_struct allocate_mem_for_production_struct() {
    productions_struct p;
    p.prod_lhs = malloc( INITIAL_SIZE * sizeof(char) );
    p.prod_rhs = malloc( INITIAL_SIZE * sizeof(char) );
    p.nonterminal_str = malloc( INITIAL_SIZE * sizeof(char) );
    p.terminal_str = malloc( INITIAL_SIZE * sizeof(char) );
    p.production_itself = malloc( INITIAL_SIZE * sizeof(char) );
    
    if (p.prod_lhs == NULL || p.prod_rhs == NULL ||
        p.nonterminal_str == NULL || p.terminal_str == NULL
        || p.production_itself == NULL) {
        
        fprintf(stderr, "%s", "Memory Allocation failed: production_lists\n");
    }
    return p;
}


/*********** Items ************/

/********* allocate Items list *********/

void allocate_item_lists() {
    item_lists = malloc( INITIAL_SIZE * sizeof(items_struct) );
    if (item_lists == NULL) {
        fprintf(stderr,"%s", "Memory Allocation failed items_lists\n");
    }
}

/***************** Reset *******************/

void reset_globals_variables() {
    tot_number_of_follows = 0;
    tot_number_of_productions = 0;
    tot_number_of_terminals = 0;
    tot_number_of_non_terminals = 0;
    tot_number_of_states = 0;
    item_list_current_pos = 0;
}

/********** check if its a starting string of follow **********/

bool check_follow_starting(char *str) {
    bool flag = false;
    for (int i = 0 ;  i < strlen(str); i++) {
        
        for (int j = 0; j < tot_number_of_terminals; j++) {
            if (terminals_list[j][0] == str[i] || str[i] == '$') {
                flag = true;
                break;
            }
        }

    }
    return flag;
}

/********** check if its a production **********/

bool check_if_a_production( char * str) {
    bool flag = false;
    for (int i = 0 ;  i < strlen(str) - 1; i++) {
        if (str[i] == '-' && str[i+1] == '>') {
            flag = true;
            break;
        }
        //str[0] = 'G'; // so string is passed by reference
    }
    return flag;
}

bool check_if_a_state(char *str) {
    if (str[0] == 'I' && str[1] >= '0' && str[1] <= '9') {
        return true;
    }
    return false;
}



/************* print to stdout ************/

void print_to_stdout() {
    
    terminals_list[tot_number_of_terminals++][0] = '$';
    terminals_list[tot_number_of_terminals][0] = '\0';
    tot_number_of_states++;
    allocate_and_initialize_print_array();
    print_define();
    build_table();
    build_reduce_arrays();
    print_action();
    print_action_num();
    print_go_to();
    print_reduce_num();
    print_reduce_lhs();
    print_tokens();
    printf("\n");
}

/*************** build necessary table for print **********/

void build_table() {
    
    for (int i = 0; i < item_list_current_pos; i++) {
        
        items_struct itm = item_lists[i];
        
        if (itm.accept) {
            action[itm.from_state][itm.transition_str_pos] = 'a';
        }
        else if (itm.shift) {
            action[itm.from_state][itm.transition_str_pos] = 's';
            action_num[itm.from_state][itm.transition_str_pos] = itm.to_state;
        } else if (itm.goto_) {
            go_to[itm.from_state][itm.transition_str_pos] = itm.to_state;
        } else if (itm.reduce) {
            
            int non_terminal_idx = itm.reduce_lhs;
            char *follow_of_ = non_terminals_list[non_terminal_idx];
            for (int i = 0; i < tot_number_of_follows; i++) {
                if (strcmp(follow_of_, follow_lists[i].non_terminal) == 0) {
                    for (int j = 0; j < strlen(follow_lists[i].terminal_str); j++) {
                        char arr[2];
                        
                        arr[0] = follow_lists[i].terminal_str[j];
                        arr[1] = '\0';
                        
                        for (int k = 0; k < TERM_NON_TERM_SIZE; k++) {
                            if (strcmp(terminals_list[k], arr) == 0) {
                                
                                action[itm.from_state][k] = 'r';
                                action_num[itm.from_state][k] = itm.reduce_production_no;
                            }
                        }
                    }
                }
            }
        }
    }
    
}

/************ build arrays for reduce operations ********/

void build_reduce_arrays() {
    for (int i = 0; i < tot_number_of_productions; i++) {
        if (i == 0) {
            reduce_lhs[i] = 0;
        }
        for (int j = 0; j < tot_number_of_non_terminals; j++) {
            if (strcmp(production_lists[i].prod_lhs, non_terminals_list[j]) == 0) {
                reduce_lhs[i] = j;
                break;
            }
        }
        reduce_num[i] = strlen(production_lists[i].prod_rhs);
    }
}

/************ allocate_and_initialize_print_array ********/

void allocate_and_initialize_print_array() {
    /*** allocate size ******/
    
    action = malloc((tot_number_of_states + EXTRA_SPACE) * sizeof(char *));
    action_num = malloc((tot_number_of_states + EXTRA_SPACE) * sizeof(int *));
    go_to = malloc((tot_number_of_states + EXTRA_SPACE) * sizeof(int *));
    reduce_lhs = malloc((tot_number_of_productions + EXTRA_SPACE) * sizeof(int));
    reduce_num = malloc((tot_number_of_productions + EXTRA_SPACE) * sizeof(int));
    
    for (int i = 0; i < tot_number_of_states + EXTRA_SPACE; i++) {
        
        action[i] = malloc((tot_number_of_terminals + EXTRA_SPACE) * sizeof(char));
        action_num[i] = malloc((tot_number_of_terminals + EXTRA_SPACE) * sizeof(int));
        go_to[i] = malloc((tot_number_of_non_terminals + EXTRA_SPACE) * sizeof(int));
        
        for (int j = 0; j < tot_number_of_terminals + EXTRA_SPACE ; j++) {
            action[i][j] = 'e';
            action_num[i][j] = 0;
        }
        for (int j = 0; j < tot_number_of_non_terminals + EXTRA_SPACE ; j++) {
            go_to[i][j] = 0;
        }
    }
}

/********* print_define **********/

void print_define() {
    printf("#define %-13s%3d\n", "NUM_STATES\0", tot_number_of_states);
    printf("#define %-13s%3d\n", "NUM_TERMS\0", tot_number_of_terminals);
    printf("#define %-13s%3d\n", "NUM_NONTERMS\0", tot_number_of_non_terminals);
    printf("#define %-13s%3d\n", "NUM_PRODS\0",tot_number_of_productions);
}


/********* print_action_array **********/

void print_action() {
    printf("\n");
    printf("static char action[NUM_STATES][NUM_TERMS] = {\n");
    printf("/*");
    for (int i = 0; i < tot_number_of_terminals; i++) {
        printf("%4s", terminals_list[i]);
        if (i < tot_number_of_terminals - 1) {
            printf(",");
        }
    }
    printf("  */\n");
    for (int i = 0; i < tot_number_of_states ; i++) {
        for (int j = 0; j < tot_number_of_terminals  ; j++) {
            if (j == 0) {
                printf("%3c",'{');
                printf("%2c%c'",'\'', action[i][j]);
            } else {
                printf("%2c%c'",'\'', action[i][j]);
            }
            if (j < tot_number_of_terminals - 1) {
                printf(",");
            }
        }
        printf("%2c",'}');
        if (i < tot_number_of_states - 1) {
            printf(",");
            printf("%3c", '/');
        }else {
            printf("%4c", '/');
        }
        printf("* %d */", i);
        printf("\n");
    }
    printf("};\n\n");
}


/********* print_action_num_array **********/

void print_action_num() {
    
    printf("\n");
    printf("static int action_num[NUM_STATES][NUM_TERMS] = {\n");
    
    printf("/*");
    for (int i = 0; i < tot_number_of_terminals; i++) {
        printf("%3s", terminals_list[i]);
        if (i < tot_number_of_terminals - 1) {
            printf(",");
        }
    }
    printf("  */\n");
    
    
    for (int i = 0; i < tot_number_of_states; i++) {
        for (int j = 0; j < tot_number_of_terminals  ; j++) {
            
            if (j == 0) {
                printf("%3c",'{');
                printf("%2d", action_num[i][j]);
            } else {
                printf("%3d", action_num[i][j]);
            }
            if (j < tot_number_of_terminals - 1) {
                printf(",");
            }
            
        }
        printf("%2c",'}');
        if (i < tot_number_of_states - 1) {
            printf(",");
            printf("%3c", '/');
        }else {
            printf("%4c", '/');
        }
        printf("* %d */", i);
        printf("\n");
    }
    printf("};\n\n");
}

/********* print_go_to_array **********/

void print_go_to() {
    
    printf("\n");
    printf("static int go_to[NUM_STATES][NUM_NONTERMS] = {\n");
    
    printf("/*");
    for (int i = 0; i < tot_number_of_non_terminals; i++) {
        printf("%3s", non_terminals_list[i]);
        if (i < tot_number_of_non_terminals - 1) {
            printf(",");
        }
    }
    printf("  */\n");
    
    for (int i = 0; i < tot_number_of_states ; i++) {
        for (int j = 0; j < tot_number_of_non_terminals  ; j++) {
            
            
            if (j == 0) {
                printf("%3c",'{');
                printf("%2d", go_to[i][j]);
            } else {
                printf("%3d", go_to[i][j]);
            }
            if (j < tot_number_of_non_terminals - 1) {
                printf(",");
            }
            
            //printf("%d, ", go_to[i][j]);
        }
        printf("%2c",'}');
        if (i < tot_number_of_states - 1) {
            printf(",");
            printf("%3c", '/');
        }else {
            printf("%4c", '/');
        }
        printf("* %d */", i);
        printf("\n");
    }
    printf("};\n\n");
}


/********* print_reduce_num_array **********/

void print_reduce_num() {
    
    printf("\n");
    printf("static int reduce_num[NUM_PRODS] =\n");
    
    printf("/*");
    for (int i = 0; i < tot_number_of_productions; i++) {
        printf("%3d", i+1);
    }
    printf("  */\n");
    printf("%3c",'{');
    for (int i = 0; i < tot_number_of_productions; i++) {
        if (!i) {
            printf("%2d", reduce_num[i]);
        } else {
            printf("%2d", reduce_num[i]);
        }
        if (i < tot_number_of_productions - 1) {
            printf(",");
        }
    }
    printf("%2c;\n",'}');
}


/********* print_reduce_lhs_array **********/

void print_reduce_lhs() {
    
    printf("\n");
    printf("static int reduce_lhs[NUM_PRODS] =\n");
    
    printf("/*");
    for (int i = 0; i < tot_number_of_productions; i++) {
        printf("%3d", i+1);
    }
    printf("  */\n");
    printf("%3c",'{');
    
    for (int i = 0; i < tot_number_of_productions; i++) {
        
        if (!i) {
            printf("%2d", reduce_lhs[i]);
        } else {
            printf("%2d", reduce_lhs[i]);
        }
        if (i < tot_number_of_productions - 1) {
            printf(",");
        }
    }
    printf("%2c;\n",'}');
}

/********* print_tokens_array **********/

void print_tokens() {
    
    printf("\n");
    printf("static char tokens[NUM_TERMS] =\n");
    printf("/*");
    for (int i = 0; i < tot_number_of_terminals; i++) {
        printf("%3d", i);
    }
    printf("  */\n");
    printf("%3c",'{');
    
    for (int i = 0; i < tot_number_of_terminals; i++) {
        
        
        if (i == 0) {
            printf("%c%s'",'\'', terminals_list[i]);
        } else {
            printf("%2c%s'",'\'', terminals_list[i]);
        }
        if (i < tot_number_of_terminals - 1) {
            printf(",");
        }
        
        //printf("%s, ", terminals_list[i]);
    }
    printf("%2c;\n",'}');
    
}




