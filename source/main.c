#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <assert.h>

int glob_id = 0; // for debugging

enum expr_type{
    EXPR_CONST, EXPR_VAR, EXPR_MULT,
    EXPR_SUM, //EXPR_DIV, EXPR_EXP
};

struct math_tree{
    int id;
    enum expr_type expr;
    struct math_tree *parts;
    double data;
    int tree_len;
};


// -----------------------------------------------
// functions' defines
// -----------------------------------------------

// string analysis functions
const char* skip_bracket_expr(const char *s);

int count_factors(const char *s);

int count_terms(const char *s);

// read functions
struct math_tree read_const(const char *s, char **p);

struct math_tree read_var(const char *s, char **p);

struct math_tree read_factor(const char *s, char **p);

struct math_tree read_mult(const char *s, char **ptr);

struct math_tree read_sum(const char *s, char **ptr);

// print functions
void print_math_tree(struct math_tree tree);

void print_tree_data(struct math_tree tree);


// -----------------------------------------------
// main function
// -----------------------------------------------

int main(void) {
    struct math_tree tree;
    char *s[]={
        "x+2", "3*x^2-2*x+3","(x+2)+1",
        "(x-2)*(x-3)*(2*x^2-3)-37*(x^2-3*x^5)",
        "(37*x^3-42*(43-8)+x+32*x)*(657*(657*879*x^2))+x^3*3"
    };
    for (int i=0; i<sizeof(s)/sizeof(*s);i++){
        tree = read_sum(s[i], NULL);
        print_math_tree(tree);
        printf("\n\n");
    }
    return 0;
}


// -----------------------------------------------
// functions' iplementations
// -----------------------------------------------

const char* skip_bracket_expr(const char *s){
    // returns pointer to ')'
    int brackets = 1;
    while (brackets && *s){
        s++;
        if (*s=='(')
            brackets++;
        else if (*s==')')
            brackets--;
    }
    assert(brackets==0 || "ERROR wrong brackets");
    return s;
}

int count_factors(const char *s){
    int factors = 1;
    
    // "-x" should be equal to "(-1)*x"
    if (s[0]=='-' && !isdigit(s[1])) factors++; 
    if (*s=='+' || *s=='-') s++;

    for (; *s!=0 && *s!=')' && *s!='+' && *s!='-'; s++) {
        if (*s=='(')
            s = skip_bracket_expr(s);
        else
            factors += (*s=='*');
    }
    return factors;
}

int count_terms(const char *s){
    int cnt = (*s!='-' && *s!='+');
    for (; *s && *s!=')'; s++) {
        if (*s=='('){
            s = skip_bracket_expr(s);
        }
        else
            cnt += (*s=='-' || *s=='+');
    }
    return cnt;
}

struct math_tree read_const(const char *s, char **p){
    return (struct math_tree){
        .expr = EXPR_CONST, .parts = NULL, 
        .tree_len = 0, .data = strtof(s, p), 
        .id=glob_id++
    };
}

struct math_tree read_var(const char *s, char **p){
    // name doesn't matter, it is skipped
    // data is power of var: "x^256" - data is 256
    while (isalnum(*++s)) ;
    *p = (char*)s;
    return (struct math_tree){
        .expr = EXPR_VAR, .parts = NULL, 
        .tree_len = 0,
        .data = (*s=='^') ? strtof(s+1, p) : 1,
        .id=glob_id++
    }; 
}

struct math_tree read_factor(const char *s, char **p){
    if (*s=='('){
        struct math_tree temp = read_sum(s+1, p);
        (*p)++; // to avoid pointing to ')'
        return temp;
    }
    if (s[0]=='-' || isdigit(s[0])) 
        return read_const(s, p);
    if (isalpha(s[0]))
        return read_var(s, p);

    printf("ERROR reading factor:\n%s\n", s);
    assert(0);
}

struct math_tree read_mult(const char *s, char **ptr){
    struct math_tree tree;
    int factors = count_factors(s);
    
    if (factors>1){
        tree = (struct math_tree){
            .tree_len = factors, .expr = EXPR_MULT,
            .parts = calloc(factors, sizeof(tree)),
            .id=glob_id++,
        };
        
        int i=0;
        // "-x" becomes (-1)*x
        if (s[0]=='-' && !isdigit(s[1])) {
            tree.parts[0] = (struct math_tree){
                .expr = EXPR_CONST, .parts = NULL, 
                .tree_len = 0, .data = -1
            };
            s++; i++;
        }
        
        char *p = (char*)s;
        for(; i<factors; i++, p++){
            tree.parts[i] = read_factor(p, &p);
        }
        if (ptr)
            *ptr = p-1; 
    }
    else{
        tree = read_factor(s, ptr);
    }
    return tree;
}

struct math_tree read_sum(const char *s, char **ptr){
    struct math_tree tree;
    int terms = count_terms(s); 
    
    if (terms>1) {
        tree = (struct math_tree){
            .expr = EXPR_SUM, .tree_len = terms,
            .parts = calloc(terms, sizeof(tree)),
            .id=glob_id++
        };
        
        int i=0;
        char *p = (char*)s;
        for(; i<terms; i++){
            tree.parts[i] = read_mult(p, &p);
            if(*p=='+') 
                p++;
        }
        if(ptr) 
            *ptr = p;
    }
    else {
        tree = read_mult(s, ptr);
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
            if (tree.parts[i].expr==EXPR_SUM)
                printf("(");
            print_math_tree(tree.parts[i]);
            if (tree.parts[i].expr==EXPR_SUM)
                printf(")");
            printf((i+1!=tree.tree_len)?"*":"");
        }
    }
    else if (tree.expr == EXPR_CONST){
        printf(tree.data>0?"%.0f":"(%.0f)", tree.data);
    }
    else if (tree.expr == EXPR_VAR){
        if (fabs(tree.data-1)>0.001)
            printf("x^%.0f", tree.data);
        else
            printf("x");
    }
}

void print_tree_data(struct math_tree tree){
    printf(
        "%d %d %d %.0f\n",
        tree.id, tree.expr, tree.tree_len, tree.data
    );
    for(int i=0; i<tree.tree_len; i++){
        print_tree_data(tree.parts[i]);
    }
}
