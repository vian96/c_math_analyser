#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>


enum expr_type{
    EXPR_CONST, EXPR_VAR, EXPR_MULT,
    EXPR_SUM,
};

struct math_tree{
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
struct math_tree read_const(const char *s, char **ptr);

struct math_tree read_var(const char *s, char **ptr);

struct math_tree read_factor(const char *s, char **ptr);

struct math_tree read_mult(const char *s, char **ptr);

struct math_tree read_sum(const char *s, char **ptr);

// print functions
void print_math_tree(struct math_tree tree);

void print_tree_data(struct math_tree tree);

// tree analysis functions
struct math_tree make_const_var(double num, double power);

void simplify_tree(struct math_tree *tree);

void free_math_tree(struct math_tree tree);


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
        printf("\n");

        simplify_tree(&tree);
        print_math_tree(tree);
        printf("\n\n");
        
        free_math_tree(tree);
    }
    return 0;
}


// -----------------------------------------------
// functions' implementations
// -----------------------------------------------


// string analysis functions

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
    assert(("ERROR: wrong brackets", brackets==0));
    return s;
}

int count_factors(const char *s){
    int factors = 1;

    // "-x" should be equal to "(-1)*x"
    if (s[0]=='-' && !isdigit(s[1])) 
        factors++;
    if (*s=='+' || *s=='-') 
        s++;

    for (; *s!=0 && *s!=')' && *s!='+' && *s!='-'; s++)
        if (*s=='(')
            s = skip_bracket_expr(s);
        else
            factors += (*s=='*');
    return factors;
}

int count_terms(const char *s){
    int cnt = (*s!='-' && *s!='+');
    for (; *s && *s!=')'; s++) 
        if (*s=='(')
            s = skip_bracket_expr(s);
        else
            cnt += (*s=='-' || *s=='+');
    return cnt;
}


// read functions

struct math_tree read_const(const char *s, char **ptr){
    return make_const_var(strtof(s, ptr), 0);
}

struct math_tree read_var(const char *s, char **ptr){
    // name doesn't matter, it is skipped
    // data is power of var: "abc^256" - data is 256
    while (isalnum(*++s)) ;
    *ptr = (char*)s;
    return make_const_var(1, (*s=='^') ? strtof(s+1, ptr) : 1);
}

struct math_tree read_factor(const char *s, char **ptr){
    if (*s=='('){
        struct math_tree temp = read_sum(s+1, ptr);
        (*ptr)++; // to avoid pointing to ')'
        return temp;
    }
    if (s[0]=='-' || isdigit(s[0]))
        return read_const(s, ptr);
    if (isalpha(s[0]))
        return read_var(s, ptr);

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
        };

        int i=0;
        // "-x" becomes "(-1)*x"
        if (s[0]=='-' && !isdigit(s[1])) {
            tree.parts[0] = make_const_var(-1, 0);
            s++; i++;
        }

        char *p = (char*)s;
        for (; i<factors; i++, p++)
            tree.parts[i] = read_factor(p, &p);

        if (ptr)
            *ptr = p-1;
    }
    else {
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
        };

        int i=0;
        char *p = (char*)s;
        for (; i<terms; i++){
            tree.parts[i] = read_mult(p, &p);
            if (*p=='+')
                p++;
        }
        if (ptr)
            *ptr = p;
    }
    else {
        tree = read_mult(s, ptr);
    }
    return tree;
}


// print functions

void print_math_tree(struct math_tree tree){
    if (tree.expr == EXPR_SUM){
        printf("(");
        for (int i=0; i<tree.tree_len; i++){
            print_math_tree(tree.parts[i]);
            printf((i+1!=tree.tree_len) ? "+" : "");
        }
        printf(")");
    }
    else if (tree.expr == EXPR_MULT){
        printf("(");
        for (int i=0; i<tree.tree_len; i++){
            print_math_tree(tree.parts[i]);
            printf((i+1!=tree.tree_len) ? "*" : "");
        }
        printf(")");
    }
    else if (tree.expr == EXPR_CONST){
        printf(tree.data>0 ? "%.0f" : "(%.0f)", tree.data);
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
            "%d %d %.0f\n",
            tree.expr, tree.tree_len, tree.data
            );
    for (int i=0; i<tree.tree_len; i++)
        print_tree_data(tree.parts[i]);
}


// tree analysis functions

struct math_tree make_const_var(double num, double power){
    struct math_tree tree = {
        .expr = EXPR_MULT, .data = 0
    };
    // TODO float comparison
    if (power && num!=1){
        tree.parts = malloc(sizeof(tree)*2);
        tree.tree_len = 2;
        tree.parts[0] = (struct math_tree){
            .expr = EXPR_CONST, .parts = 0,
            .data = num, .tree_len = 0
        };
        tree.parts[1] = (struct math_tree){
            .expr = EXPR_VAR, .parts = 0,
            .data = power, .tree_len = 0
        };
    }
    else if (power)
        tree = (struct math_tree){
            .expr = EXPR_VAR, .parts = 0,
            .data = power, .tree_len = 0
        };
    else
        tree = (struct math_tree){
            .expr = EXPR_CONST, .parts = 0,
            .data = num, .tree_len = 0
        };
    return tree;
}

void simplify_tree(struct math_tree *tree){
    for (int i=0; i<tree->tree_len; i++)
        simplify_tree(tree->parts+i);
    if (tree->expr == EXPR_SUM || tree->expr == EXPR_MULT){
        // (x+1)+2 => x+1+2
        int new_len = tree->tree_len;
        for (int i=0; i<tree->tree_len; i++)
            if (tree->parts[i].expr == tree->expr)
                new_len += tree->parts[i].tree_len - 1;

        if (new_len != tree->tree_len){
            tree->parts = realloc(
                tree->parts, sizeof(*tree)*new_len
            );

            // TODO make it more beautiful
            for (int i=0, n=tree->tree_len; i<tree->tree_len; i++)
                if (tree->parts[i].expr == tree->expr){
                    for (int k=1; k < tree->parts[i].tree_len; k++)
                        tree->parts[n++] = tree->parts[i].parts[k];
                    void *t=tree->parts[i].parts;
                    tree->parts[i] = tree->parts[i].parts[0];
                    free(t);
                }

            tree->tree_len = new_len;
        }
    }
}

void free_math_tree(struct math_tree tree){
    for (int i=0; i<tree.tree_len; i++)
        free_math_tree(tree.parts[i]);
    free(tree.parts);
}