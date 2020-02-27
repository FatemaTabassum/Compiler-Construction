/****************************************************************
 *                                                              *
 *      Name: Fatema Tabassum Liza                              *
 *      Class: COP5621                                          *
 *      Assignment: Asg 3 (Implementing a C Parser)            *
 *      Compile: "gcc -std=c99 cparse.c"                       *
 *                                                              *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "tables.h"


/***************** Macros ***************/
#define INPUT_SIZE 110
#define STACK_OVERFLOW_LIMIT 100

char input_str[INPUT_SIZE];

/*************** Enums and Structs *****************/
typedef struct stack_ {
    bool is_state_num;
    bool is_go_to;
    char token;
    int state_num;
    int go_to_num;
} stack_struct;


/**************** Function Prototypes **********/
void push_to_stack(stack_struct val);
void pop(void);
void print_stack_element(void);
int get_token_id(char tok);
void increase_input_str_pos(void);
void initialize_stack(void);
void reset_values(void);
void action_reduce(int state_no, int idx);
void action_shift(int state_no, int idx);

/****************** Global Variables **********/
int input_str_pos;
int stack_top;
int shift_to_state;
int reduce_production;
stack_struct current;
stack_struct* stack_list;
stack_struct default_val;


/************** main ***************/
int main() {
    
    stack_list = malloc(INPUT_SIZE * sizeof(stack_struct));
    while (fgets(input_str, INPUT_SIZE, stdin)) {
        
        reset_values();
        input_str[strlen(input_str)] = '\0';
        input_str[strlen(input_str) - 1] = '$';
        while (true) {
            
            if (input_str_pos > strlen(input_str)) {
                break;
            }
            int idx = get_token_id(input_str[input_str_pos]);
            if (idx <= -1) {
                increase_input_str_pos();
                continue;
            }
            int state_no = stack_list[stack_top].state_num;
            char action_ch = action[state_no][idx];
            if (action_ch == 's') {
                action_shift(state_no, idx);
                
            } else if (action_ch == 'r') {
                action_reduce(state_no, idx);
                
            }
            else if (action_ch == 'a') {
                printf("\nAccept state reached.\n\n");
                break;
            } else if (action_ch == 'e') {
                fprintf(stderr, "\nError state on token '%c' at state %d.\n\n",
                        input_str[input_str_pos], state_no );
                break;
            } else if(stack_top > STACK_OVERFLOW_LIMIT + 1) {
                fprintf(stderr, "%s", "\nStack overflow occured.\n\n" );
                //printf("\nStack overflow occured.\n\n");
                break;
            }
        }
    }
}


/******* function on action shift ********/
void action_shift(int state_no, int idx) {
    //printf("shift %d\n",action_num[state_no][idx]);
    //push
    current.is_state_num = false;
    current.is_go_to = false;
    current.token = input_str[input_str_pos];
    push_to_stack(current);
    
    increase_input_str_pos();
    shift_to_state = action_num[state_no][idx];
    
    // push
    current.is_state_num = true;
    current.is_go_to = false;
    current.state_num = shift_to_state;
    push_to_stack(current);
    
    //print_stack_element();
}

/******* function on action reduce ********/
void action_reduce(int state_no, int idx) {
    reduce_production = action_num[state_no][idx];
    printf("reduce %d\n", reduce_production);
    int pop_num = reduce_num[reduce_production];
    for (int j = 0; j < pop_num * 2; j++) {
        pop();
    }
    //print_stack_element();
    int left_symbol_of_prod = reduce_lhs[reduce_production];
    
    //push
    current.is_go_to = true;
    current.is_state_num = false;
    current.go_to_num = left_symbol_of_prod;
    push_to_stack(current);
    
    int prev_state_no = stack_list[stack_top-1].state_num;
    int new_state_no = go_to[prev_state_no][left_symbol_of_prod];
    
    //push
    current.is_go_to = false;
    current.is_state_num = true;
    current.state_num = new_state_no;
    push_to_stack(current);
    
    //print_stack_element();
}

/******* reset initial values to each input ***********/
void reset_values() {
    input_str_pos = 0;
    stack_top = 0;
    shift_to_state = 0;
    initialize_stack();
}

/*********** initialize stack **********/
void initialize_stack() {
    
    stack_struct val;

    default_val.state_num = -1;
    default_val.is_state_num = false;
    default_val.is_go_to = false;
    default_val.go_to_num = -1;
    stack_list[stack_top] = default_val;
    
    val.state_num = 0;
    val.is_state_num = true;
    val.is_go_to = false;
    val.go_to_num = -1;
    
    push_to_stack(val);

    
}

/******* print *******/
void print_stack_element() {
    for (int i = 0; i < stack_top + 1; i++) {
        if (stack_list[i].is_state_num) {
            printf("%d, ",stack_list[i].state_num);
        } else if(stack_list[i].is_go_to) {
            printf("%d, ",stack_list[i].go_to_num);
        } else {
            printf("%c, ",stack_list[i].token);
        }
    }
    printf("\n");
}

/******* push to stack *******/
void push_to_stack(stack_struct val) {
    stack_list[++stack_top] = val;
}

/******* pop from stack *******/
void pop() {
    stack_list[stack_top] = default_val;
    stack_top--;
}

void increase_input_str_pos() {
    input_str_pos++;
}

/******* return token index from tokens array *******/
int get_token_id(char tok) {
    
    for (int i = 0 ; i < NUM_TERMS; i++) {
        if (tokens[i] == tok) {
            return i;
        }
    }
    return  - 1;
}
