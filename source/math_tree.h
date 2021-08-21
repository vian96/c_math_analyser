#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>


typedef enum expr_type expr_type;

enum expr_type {
    EXPR_CONST = 0, EXPR_VAR = 1,
    EXPR_MULT = 2, EXPR_SUM = 3
};

typedef struct math_tree math_tree;

struct math_tree {
    expr_type type;
    math_tree *parts;
    double data;
    int tree_len;
};


// read functions
math_tree read_sum (const char *str, char **ptr);

math_tree read_mult (const char *str, char **ptr);

math_tree read_factor (const char *str, char **ptr);

math_tree read_var (const char *str, char **ptr);

math_tree read_const (const char *str, char **ptr);


// string analysis functions
int count_terms (const char *str);

int count_factors (const char *str);

const char* skip_bracket_expr (const char *str);


// print functions
void print_math_tree (math_tree tree);

void print_tree_data (math_tree tree);

// working with tree
math_tree create_monomial (double num, double power);

void simplify_tree (math_tree *tree);

void free_math_tree (math_tree tree);

