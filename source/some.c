#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <assert.h>

int glob_id = 0;

enum expr_type{
    EXPR_CONST, EXPR_VAR, EXPR_MULT,
    EXPR_SUM, EXPR_DIV, EXPR_EXP
};

struct math_tree{
    int id;
    enum expr_type expr;
    struct math_tree *parts;
    double data;
    int tree_len, data_len;
};

int count_pm(const char *s){
    int cnt=0;
    for(int i=0; s[i]!=0; i++) cnt += (s[i]=='-' || s[i]=='+');
    return cnt;
}

struct math_tree read_number(const char *s){
    struct math_tree tree = {
        .expr = EXPR_CONST, .parts=NULL, .tree_len=0, .data_len=1, .data=atof(s), .id=glob_id++
    };
    return tree; 
}

struct math_tree read_var(const char *s){
    struct math_tree tree = {
        .expr = EXPR_VAR, .parts=NULL, .tree_len=0, .data_len=1, .data=(s[1]=='^')?atof(s+2):1,
         .id=glob_id++
    }; // data is power of x: "x^256" - data is 256
    return tree;
}

struct math_tree read_factor(const char *s){
    if (s[0]=='-' || isdigit(s[0])) return read_number(s);
    else if (isalpha(s[0])) return read_var(s);
    else {printf("%s\n", s);assert(0);}
    // TODO EXPONENT 
}

struct math_tree read_mult(const char *s, int *end){
    //printf("%s\n", s);
    struct math_tree tree = {
        .tree_len=0, .data_len=0, .expr=EXPR_MULT, .id=glob_id++
    };
    
    int factors=1, i=0, n=0;
    if (s[i]=='+') i++;
    for (int i=0; s[i]!=0 && (s[i]!='+' && s[i]!='-' || i==0); i++) factors += (s[i]=='*');
    if (s[0]=='-' && !isdigit(s[1])) factors++; // "-x" should be equal to "(-1)*x"
    
    if (factors>1){
        tree.parts = calloc(factors, sizeof(tree));
        tree.tree_len = factors;
        
        if(s[0]=='-' && !isdigit(s[1])) {
            struct math_tree temp = {
                .expr = EXPR_CONST, .parts=NULL, .tree_len=0, .data_len=1, .data=-1
            }; 
            tree.parts[0] = temp;
            i++; n++;
        }
        
        for(; n<factors; n++, i++){  
            tree.parts[n] = read_factor(s+i);
            //printf("%s %d %d\n", s+i, n, factors);
            if (n+1<factors) for(;s[i]!='*';i++);
            //printf("%s\n", s+i);
        }
    }
    else{
        tree = read_factor(s+i);
    }
    if (end) {
        for(;s[i]=='*' || s[i]=='^' || isalnum(s[i]);i++);
        *end += i;
    }
    return tree;
}

struct math_tree read_poly(const char *s){
    struct math_tree tree = {
        .tree_len=0, .data_len=0, .expr=EXPR_SUM, .id=glob_id++
    }, temp_tree;
    int terms = count_pm(s+1); // s+1 to avoid first minus (e.g. "-1+2" will return 2)
    
    if (terms) {
        terms++;
        tree.parts = calloc(terms, sizeof(tree));
        tree.tree_len = terms;
        
        for(int i=0, p=0; i<terms; i++ /*,p++*/){
            tree.parts[i] = read_mult(s+p, &p);
            //printf("%d\n", p);
        }  
    }
    else {
        tree = read_mult(s, NULL);
    }
    return tree;
}

void print_math_tree(struct math_tree tree){
    if (tree.expr == EXPR_SUM){
        for(int i=0; i<tree.tree_len; i++){
            print_math_tree(tree.parts[i]);
            printf((i+1!=tree.tree_len)?"+":"");
        }
    }
    else if (tree.expr == EXPR_MULT){
        for(int i=0; i<tree.tree_len; i++){
            print_math_tree(tree.parts[i]);
            printf((i+1!=tree.tree_len)?"*":"");
        }
    }
    else if (tree.expr == EXPR_CONST){
        printf(tree.data>0?"%.3f":"(%.3f)", tree.data);
    }
    else if (tree.expr == EXPR_VAR){
        if (fabs(tree.data-1)>0.001)
            printf("x^%.3f", tree.data);
        else
            printf("x");
    }
}

void print_tree_data(struct math_tree tree){
    printf("%d %d %d %.3f\n", tree.id, tree.expr, tree.tree_len, tree.data);
    for(int i=0; i<tree.tree_len; i++){
        print_tree_data(tree.parts[i]);
    }
}


int main(void) {
    char s[]={"3*x^2-2*x+3"};
    struct math_tree tree = read_poly(s);

    //print_tree_data(tree);
    print_math_tree(tree);
    printf("\n\n");
    return 0;
}