# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "scan.h"
# include "cc.h"
# include "semutil.h"
# include "sem.h"
# include "sym.h"

# define MAXLOOPSTK 50

extern int formalnum;
extern char formaltypes[];
extern int localnum;
extern char localtypes[];

/*
*function prototype
*/
struct sem_rec * set_temp(struct sem_rec * t, int s_place, int s_mode);
int next_label_no(void);
int current_label_no(void);
int next_temp_label_no(void);
int cur_temp_label_no(void);
struct sem_rec * opConv(char *op, struct sem_rec *x1, struct sem_rec *y);
char * string_to_val(int v);
void save_loop_rec(struct sem_rec *s);
void new_loop(void);

/*
* Globals
*/
int label_no = 0;
int temp_label_no = 0;
int numlooprecs = 0;
struct sem_rec *loopstk[MAXLOOPSTK];               /* stack of ptrs to semantic recs representing loop */
struct sem_rec **looptop = loopstk;                /* stack pointer */
struct sem_rec **prevlooptop = NULL;               /* previous top */
char * labels[55];
int tot_labels = 0;

int next_label_no() {
    return ++label_no;
}

int current_label_no() {
    return label_no;
}

int next_temp_label_no() {
    return  ++temp_label_no;
}

int cur_temp_label_no() {
    return temp_label_no;
}

char * string_to_val(int v) {
    char *s = (char *) malloc(sizeof(char) * 100);
    if (v == T_INT) {
        strcpy(s, "i\0");
        return s;
    } else if (v == T_DOUBLE) {
        strcpy(s, "f\0");
        return s;
    } else {
        printf("not defined string to val\n");
        return NULL;
    }
}

struct sem_rec * set_temp(struct sem_rec * t, int s_place, int s_mode) {
    t->s_mode = s_mode;
    t->s_place = s_place;
    return t;
}

struct sem_rec * opConv(char *op, struct sem_rec *x1, struct sem_rec *y) {
    struct sem_rec *x = op1("@", x1);
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t->s_place = nexttemp();
    t->s_mode = x->s_mode;
    printf("t%d := t%d %s%s t%d\n",t->s_place, x->s_place, op, string_to_val(t->s_mode), y->s_place);
    return ((struct sem_rec *) t);
}

void new_loop()
{
    save_loop_rec((struct sem_rec *) prevlooptop);
    prevlooptop = looptop - 1;
}

void save_loop_rec(struct sem_rec *s)
{
    /* save on stack so can reclaim */
    if (numlooprecs++ > MAXLOOPSTK) {
        fprintf(stderr, "too many semantic records\n");
        exit(1);
    }
    *looptop++ = s;
}






/*
 * backpatch - backpatch list of quadruples starting at p with k
 */
void backpatch(struct sem_rec *p, int k)
{
   if (p == NULL) {
        p = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        p->s_place = k;
    }
    struct sem_rec *pp;
    for (pp = p; pp->back.s_link; pp = pp->back.s_link)
        ;
    printf("B%d=L%d\n", pp->s_place, k);
    pp->s_place = k;
}

/*
 * bgnstmt - encountered the beginning of a statement
 */
void bgnstmt()
{
   extern int lineno;
   skip();
   printf("bgnstmt %d\n", lineno);
   //fprintf(stderr, "sem: bgnstmt not implemented\n");
}

/*
 * call - procedure invocation
 */
struct sem_rec *call(char *f, struct sem_rec *args)
{
    struct sem_rec *p;
    char *int_ = "i\0";
    char *f_ = "f\0";
    int cnt = 0;
    int global_block_level = 2;
    for (p = args; p->back.s_link; p = p->back.s_link) {
        cnt++;
        if (p->s_mode == T_INT) {
            printf("arg%s t%d\n", int_, p->s_place);
        } else if (p->s_mode == T_DOUBLE) {
            printf("arg%s t%d\n", f_, p->s_place);
        }
    }
    if (p->s_mode == T_INT) {
        printf("arg%s t%d\n", int_, p->s_place);
    } else if (p->s_mode == T_DOUBLE) {
        printf("arg%s t%d\n", f_, p->s_place);
    }
    cnt++;
    printf("t%d := global %s\n",nexttemp(), f);
    struct id_entry *fname = lookup(f, global_block_level);
    char type[10];
    if (!fname) {
        strcpy(type, int_);
    } else {
        sprintf(type, "%d", fname->i_type);
    }
    printf("t%d := f%s t%d %d\n", nexttemp(), int_, currtemp()-1, cnt );
    return ((struct sem_rec *) args);
   //return ((struct sem_rec *) NULL);
}

/*
 * ccand - logical and
 */
struct sem_rec *ccand(struct sem_rec *e1, int m, struct sem_rec *e2)
{
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t = set_temp(t, nexttemp(), T_INT);
    struct sem_rec * mrg = merge(e1->s_false, e2->s_false);
    backpatch(e1->back.s_true, m);
    backpatch(t->back.s_true, e2->back.s_true->s_place);
    backpatch(t->s_false, mrg->s_place);
    return ((struct sem_rec *) t);
}

/*
 * ccexpr - convert arithmetic expression to logical expression
 */
struct sem_rec *ccexpr(struct sem_rec *e)
{
    e->back.s_true = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    e->back.s_true->s_place = next_temp_label_no();
    e->s_false = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    e->s_false->s_place = next_temp_label_no();
    printf("bt t%d B%d\n", e->s_place, e->back.s_true->s_place);
    printf("br B%d\n", e->s_false->s_place);
    return ((struct sem_rec *) e);
}

/*
 * ccnot - logical not
 */
struct sem_rec *ccnot(struct sem_rec *e)
{
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t = set_temp(t, nexttemp(), T_INT);
    backpatch(t->back.s_true, e->s_false->s_place);
    backpatch(t->s_false, e->back.s_true->s_place);
    return ((struct sem_rec *) t);
}

/*
 * ccor - logical or
 */
struct sem_rec *ccor(struct sem_rec *e1, int m, struct sem_rec *e2)
{
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t = set_temp(t, nexttemp(), T_INT);
    struct sem_rec * mrg = merge(e1->back.s_true, e2->back.s_true);
    backpatch(e1->s_false, m);
    backpatch(t->back.s_true, mrg->s_place);
    backpatch(t->s_false, e2->s_false->s_place);
    return ((struct sem_rec *) t);
}

/*
 * con - constant reference in an expression
 */
struct sem_rec *con(char *x)
{
  struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t->s_place = nexttemp();
    int i;
    int flag = 0;
    for (i = 0; i < strlen(x); i++) {
        if (x[i] == '.') {
            flag = 1;
            break;
        }
    }
    if (flag) {
        t->s_mode = T_DOUBLE;
    } else {
        t->s_mode = T_INT;
    }
    
    printf("t%d := %s\n",currtemp(), x);
    return ((struct sem_rec *) t);
}

/*
 * dobreak - break statement
 */
void dobreak()
{
   if ((*looptop) && (*looptop)->s_false) {
        
    } else {
        (*looptop) = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        (*looptop)->s_false = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        (*looptop)->s_false->s_place = next_temp_label_no(); // keeping  track of start for dobreak
    }
    printf("br B%d\n", (*looptop)->s_false->s_place);
}

/*
 * docontinue - continue statement
 */
void docontinue()
{
    if ((*looptop) && (*looptop)->back.s_link) {
        
    } else {
        (*looptop) = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        (*looptop)->back.s_link = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        (*looptop)->back.s_link->s_place = next_temp_label_no(); // keeping  track of start for dobreak
    }
    printf("br B%d\n", (*looptop)->back.s_link->s_place);
}

/*
 * dodo - do statement
 */
void dodo(int m1, int m2, struct sem_rec *e, int m3)
{
    backpatch(e->back.s_true, m1);
    backpatch(e->s_false, m3);
    struct sem_rec *p = (*looptop);
    if (p && p->back.s_link) {
        backpatch((*looptop)->back.s_link, m2); //  // keeping  track  for docontinue
    }
    if (p && p->s_false) {
        backpatch((*looptop)->s_false, m3);  // keeping  track for dobreak
    }
    endloopscope(m3);
}

/*
 * dofor - for statement
 */
void dofor(int m1, struct sem_rec *e2, int m2, struct sem_rec *n1,
           int m3, struct sem_rec *n2, int m4)
{
   backpatch(e2->back.s_true, m3);
   backpatch(e2->s_false, m4);
   backpatch(n1, m1);
   backpatch(n2, m2);
   struct sem_rec *p = (*looptop);
   if (p && p->back.s_link) {
        backpatch((*looptop)->back.s_link, m2); // keeping  track for docontinue
   }
   if (p && p->s_false) {
        backpatch((*looptop)->s_false, m4);  // keeping  track for dobreak
   }
    endloopscope(m4);
}

/*
 * dogoto - goto statement
 */
void dogoto(char *id)
{
int i;
    int flag = 0;
    for (i = 0; i < tot_labels; i++) {
        if((strcmp(labels[i], id) == 0)) {
            flag = 1;
            break;
        }
    }
    if (flag == 1) {
        printf("br %s\n", labels[i]);
    } else {
        fprintf(stderr, "label is not declared\n");
        exit(1);
    }
}

/*
 * doif - one-arm if statement
 */
void doif(struct sem_rec *e, int m1, int m2)
{
   backpatch(e->back.s_true, m1);
   backpatch(e->s_false, m2);
}

/*
 * doifelse - if then else statement
 */
void doifelse(struct sem_rec *e, int m1, struct sem_rec *n,
                         int m2, int m3)
{
   backpatch(e->back.s_true, m1);
   backpatch(n, m3);
   backpatch(e->s_false, m2);
}

/*
 * doret - return statement
 */
void doret(struct sem_rec *e)
{
    if (e != NULL) {
        printf("ret%s t%d\n", string_to_val(e->s_mode), e->s_place);
    } else {
        printf("ret%s\n", string_to_val(T_INT));
    }
}

/*
 * dowhile - while statement
 */
void dowhile(int m1, struct sem_rec *e, int m2, struct sem_rec *n,
             int m3)
{
   backpatch(e->back.s_true, m2);
   backpatch(e->s_false, m3);
   backpatch(n, m1);
   struct sem_rec *p = (*looptop);
   if (p && p->back.s_link) {
        backpatch((*looptop)->back.s_link, m1); // keeping  track  for docontinue
   }
   if (p && p->s_false) {
        backpatch((*looptop)->s_false, m3);  // keeping  track for dobreak
   }
   endloopscope(m3);
}

/*
 * endloopscope - end the scope for a loop
 */
void endloopscope(int m)
{
   //printf("br L%d\n", m);
    for (looptop--; looptop > prevlooptop;) {
        numlooprecs--;
        free((char *) *looptop--);
    }
    prevlooptop = (struct sem_rec **) *looptop;
}

/*
 * exprs - form a list of expressions
 */
struct sem_rec *exprs(struct sem_rec *l, struct sem_rec *e)
{
   struct sem_rec * new_list = merge(l, e);
   return ((struct sem_rec *) new_list);
   //return ((struct sem_rec *) NULL);
}

/*
 * fhead - beginning of function body
 */
void fhead(struct id_entry *p)
{
  int i;
    for(i = 0; i< formalnum; i++) {
        if (formaltypes[i] == 'f') {
            printf("formal %d\n", tsize(T_DOUBLE));
        } else if (formaltypes[i] == 'i') {
            printf("formal %d\n", tsize(T_INT));
        }
    }
    for(i = 0; i< localnum; i++) {
        if (localtypes[i] == 'f') {
            printf("localloc %d\n", tsize(T_DOUBLE));
        } else if (localtypes[i] == 'i') {
            printf("localloc %d\n", tsize(T_INT));
        }
    }
}

/*
 * fname - function declaration
 */
struct id_entry *fname(int t, char *id)
{
   int return_type_of_func = t;
    printf("func %s\n", id);
    struct id_entry *p;
    extern int level;
    char msg[80];
    int global_block_level = 2;
    enterblock();

    if ((p = lookup(id, 0)) == NULL || p->i_blevel != level)
        p = install(id, global_block_level);
    else  {
        sprintf(msg, "function %s previously declared", id);
        yyerror(msg);
        return (p);
    }
    p->i_defined = 1;
    p->i_type = return_type_of_func;
   //fprintf(stderr, "sem: fname not implemented\n");
   return (p);
}

/*
 * ftail - end of function body
 */
void ftail()
{
   printf("fend\n");
   leaveblock();
}

/*
 * id - variable reference
 */
struct sem_rec *id(char *x)
{
   struct sem_rec *t;
    struct id_entry *id = lookup(x, 0);
    
    if (id == NULL) {
        char msg[50];
        sprintf(msg, "variable %s not declared", x);
        yyerror(msg);
        return ((struct sem_rec *)id);
    }

    /* allocate space */
    t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    
    t->s_place = nexttemp();
    t->s_mode = id->i_type;
    char levl[15];
    if (id->i_scope == GLOBAL) {
        strcpy(levl,"global" );
    } else if (id->i_scope == PARAM) {
        strcpy(levl,"param" );
    } else if (id->i_scope == LOCAL) {
        strcpy(levl,"local" );
    } else {
        strcpy(levl,"-1" );
    }
    if (id->i_scope == GLOBAL) {
        printf("t%d := %s %s\n",currtemp(), levl, id->i_name);
    } else {
        printf("t%d := %s %d\n",currtemp(), levl, id->i_offset);
    }
    return (t);
   //return (struct sem_rec *)NULL;
}

/*
 * index - subscript
 */
struct sem_rec *index(struct sem_rec *x, struct sem_rec *i)
{
   struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
   t->s_place = nexttemp();
   t->s_mode = (x->s_mode & ~T_ARRAY);
   printf("t%d := t%d []%s t%d\n", t->s_place, x->s_place, string_to_val(t->s_mode), i->s_place);
   return ((struct sem_rec *) t);
}

/*
 * labeldcl - process a label declaration
 */
void labeldcl(char *id)
{
   /* you may assume the maximum number of C label declarations is 50 */
    if (tot_labels > 50) {
        fprintf(stderr, "too many labels\n");
        exit(1);
    } else {
        labels[tot_labels] = (char *) malloc(50 * sizeof(char));
        strcpy(labels[tot_labels], id);
        printf("label %s:\n", id);
        tot_labels++;
    }
}

/*
 * m - generate label and return next temporary number
 */
int m()
{
   int i = next_label_no();
   printf("label L%d\n", i);
   return current_label_no();
}

/*
 * n - generate goto and return backpatch pointer
 */
struct sem_rec *n()
{
   struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
   t->s_place =  next_temp_label_no();
   printf("br B%d\n", cur_temp_label_no());
   return ((struct sem_rec *) t);
}

/*
 * op1 - unary operators
 */
struct sem_rec *op1(char *op, struct sem_rec *y)
{
   /* allocate space */
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t->s_place = nexttemp();
    char type[5];
    if (y->s_mode == T_INT) {
        strcpy(type, "i");
    } else {
        strcpy(type, "f");
    }
    printf("t%d := %s%s t%d\n", currtemp(), op, type, y->s_place);
    t->s_mode = y->s_mode;
    return ((struct sem_rec *) t);
}

/*
 * op2 - arithmetic operators
 */
struct sem_rec *op2(char *op, struct sem_rec *x, struct sem_rec *y)
{
   struct sem_rec *t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    struct sem_rec * t2 = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    
    if (x->s_mode == T_INT && y->s_mode == T_DOUBLE) {
        t->s_mode = T_DOUBLE;
        t->s_place = nexttemp();
        printf("t%d := cv%s t%d\n", t->s_place , string_to_val(t->s_mode), x->s_place );
        t2->s_place = nexttemp();
        t2->s_mode = t->s_mode;
        printf("t%d := t%d %s%s t%d\n", t2->s_place, t->s_place, op, string_to_val(t2->s_mode), y->s_place);
    } else if (x->s_mode == T_DOUBLE && y->s_mode == T_INT) {
        t->s_mode = T_DOUBLE;
        t->s_place = nexttemp();
        printf("t%d := cv%s t%d\n", t->s_place, string_to_val(t->s_mode), y->s_place );
        t2->s_place = nexttemp();
        t2->s_mode = t->s_mode;
        printf("t%d := t%d %s%s t%d\n", t2->s_place, x->s_place, op, string_to_val(t2->s_mode), t->s_place );
    } else {
        t2->s_mode = x->s_mode;
        t2->s_place = nexttemp();
        printf("t%d := t%d %s%s t%d\n", t2->s_place, x->s_place, op, string_to_val(t2->s_mode), y->s_place );
    }
    return ((struct sem_rec *) t2);
}

/*
 * opb - bitwise operators
 */
struct sem_rec *opb(char *op, struct sem_rec *x, struct sem_rec *y)
{
    struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    if (x->s_mode == T_INT && y->s_mode == T_INT) {
        t = set_temp(t, nexttemp(), T_INT);
        printf("t%d := t%d %s%s t%d\n",t->s_place, x->s_place, op, string_to_val(t->s_mode), y->s_place);
    } else {
        fprintf(stderr,"Operator type error\n" );
        exit(1);
    }
    return ((struct sem_rec *) t);
}

/*
 * rel - relational operators
 */
struct sem_rec *rel(char *op, struct sem_rec *x, struct sem_rec *y)
{
   struct sem_rec * t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    struct sem_rec * t2 = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    if (x->s_mode == T_INT && y->s_mode == T_DOUBLE) {
        t->s_mode = T_DOUBLE;
        t->s_place = nexttemp();
        printf("t%d := cv%s t%d\n", t->s_place , string_to_val(t->s_mode), x->s_place );
        t2->s_place = nexttemp();
        t2->s_mode = t->s_mode;
        printf("t%d := t%d %s%s t%d\n", t2->s_place, t->s_place, op, string_to_val(t2->s_mode), y->s_place);
    } else if (x->s_mode == T_DOUBLE && y->s_mode == T_INT) {
        t->s_mode = T_DOUBLE;
        t->s_place = nexttemp();
        printf("t%d := cv%s t%d\n", t->s_place, string_to_val(t->s_mode), y->s_place );
        t2->s_place = nexttemp();
        t2->s_mode = t->s_mode;
        printf("t%d := t%d %s%s t%d\n", t2->s_place, x->s_place, op, string_to_val(t2->s_mode), t->s_place );
    } else {
        t2->s_mode = x->s_mode;
        t2->s_place = nexttemp();
        printf("t%d := t%d %s%s t%d\n", t2->s_place, x->s_place, op, string_to_val(t2->s_mode), y->s_place );
    }
    t2->back.s_true = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t2->back.s_true->s_place = next_temp_label_no();
    t2->s_false = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t2->s_false->s_place = next_temp_label_no();
    printf("bt t%d B%d\n", t2->s_place, t2->back.s_true->s_place);
    printf("br B%d\n", t2->s_false->s_place);
    return ((struct sem_rec *) t2);
}

/*
 * set - assignment operators
 */
struct sem_rec *set(char *op, struct sem_rec *x, struct sem_rec *y)
{
   struct sem_rec * t, * t1, *t2;
    if (x->s_mode != y->s_mode) {
        t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
        t->s_place = nexttemp();
        t->s_mode = x->s_mode;
        printf("t%d := cv%s t%d\n", t->s_place, string_to_val(t->s_mode), y->s_place);
    } else {
        t = y;
    }
    
    t1 =  (struct sem_rec *) alloc(sizeof(struct sem_rec));
    t2 =  (struct sem_rec *) alloc(sizeof(struct sem_rec));

    if ( strcmp(op, "") == 0) {
        t1->s_place = nexttemp();
        t1->s_mode = x->s_mode;
        printf("t%d := t%d %s%s t%d\n", t1->s_place, x->s_place, "=", string_to_val(t1->s_mode), t->s_place );
        t2 = t1;
    } else {
        t1 = opConv(op, x, t);
        t2->s_place = nexttemp();
        t2->s_mode = t1->s_mode;
        printf("t%d := t%d %s%s t%d\n", t2->s_place, x->s_place, "=", string_to_val(t1->s_mode), t1->s_place );
    }
    
   //fprintf(stderr, "sem: set not implemented\n");
   return ((struct sem_rec *) t2);
}

/*
 * startloopscope - start the scope for a loop
 */
void startloopscope()
{
    /* you may assume the maximum number of loops in a loop nest is 50 */
    new_loop();
}

/*
 * string - generate code for a string
 */
struct sem_rec *string(char *s)
{
   struct sem_rec *t;
    
    /* allocate space */
    t = (struct sem_rec *) alloc(sizeof(struct sem_rec));
    
    t->s_place = nexttemp();
    t->s_mode = T_INT;
    printf("t%d := %s\n",currtemp(), s);
    
    return (t);
    //return ((struct sem_rec *) NULL);

}
