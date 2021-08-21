#include "math_tree.h"


// read functions

math_tree read_sum (const char *str, char **ptr) {
    assert (str);
    math_tree tree;
    int terms = count_terms (str);

    if (terms > 1) {
        tree = (math_tree) {
            .type = EXPR_SUM, .tree_len = terms,
            .parts = calloc (terms, sizeof (tree)),
        };

        for (int i = 0; i < terms; i++) {
            tree.parts[i] = read_mult (str, &str);
            if (*str == '+')
                str++;
        }
        if (ptr)
            *ptr = str;
    }
    else {
        tree = read_mult (str, ptr);
    }
    return tree;
}

math_tree read_mult (const char *str, char **ptr) {
    assert (str);
    math_tree tree;
    int factors = count_factors (str);

    if (factors > 1) {
        tree = (math_tree) {
            .tree_len = factors, .type = EXPR_MULT,
            .parts = calloc (factors, sizeof (tree)),
        };

        int i = 0;
        // "-x" becomes "(-1)*x"
        if (str[0] == '-' && !isdigit (str[1])) {
            tree.parts[0] = create_monomial (-1, 0);
            str++; 
            i++;
        }

        for (; i < factors; i++, str++)
            tree.parts[i] = read_factor (str, &str);

        if (ptr)
            *ptr = str-1;
    }
    else {
        tree = read_factor (str, ptr);
    }
    return tree;
}

math_tree read_factor (const char *str, char **ptr) {
    assert (str);
    if (*str == '(') {
        math_tree temp = read_sum (str+1, ptr);
        (*ptr)++; // to avoid pointing to ')'
        return temp;
    }
    if (str[0] == '-' || isdigit (str[0]))
        return read_const (str, ptr);
    if (isalpha (str[0]))
        return read_var (str, ptr);

    // TODO returning error
    printf ("ERROR reading factor:\n%s\n", str);
    assert(0);
}

math_tree read_var (const char *str, char **ptr) {
    // name doesn't matter, it is skipped
    // data is power of var: "abc^256" - data is 256
    // TODO read var name
    assert (str);
    while (isalnum (*str)) 
        str++;
    
    if (*str == '^')
        return create_monomial (1, strtof (str + 1, ptr)); 
    else {
        *ptr = (char*) str;
        return create_monomial (1, 1);
    }
}

math_tree read_const (const char *str, char **ptr) {
    assert (str);
    return create_monomial (strtof (str, ptr), 0);
}


// string analysis functions

int count_terms (const char *str) {
    assert (str);
    int cnt = (*str != '-' && *str != '+');
    for (; *str != 0 && *str != ')'; str++) 
        if (*str == '(')
            str = skip_bracket_expr (str);
        else
            cnt += (*str == '-' || *str == '+');
    return cnt;
}

int count_factors (const char *str) {
    assert (str);
    int factors = 1;

    // "-x" should be equal to "(-1)*x"
    if (str[0] == '-' && !isdigit (str[1])) 
        factors++;
    if (*str == '+' || *str == '-') 
        str++;

    for (; *str != 0 && *str != ')' && *str != '+' && *str != '-'; str++)
        if (*str == '(')
            str = skip_bracket_expr (str);
        else
            factors += (*str == '*');
    return factors;
}

const char* skip_bracket_expr (const char *str) {
    // returns pointer to ')'
    // if '(' more than ')' returns pointer to '\0'
    assert (str);
    int brackets = 1;
    while (brackets && *str != 0) {
        str++;
        if (*str == '(')
            brackets++;
        else if (*str == ')')
            brackets--;
    }
    return str;
}


// print functions

void print_math_tree (math_tree tree) {
    if (tree.type == EXPR_SUM) {
        printf ("(");
        for (int i=0; i < tree.tree_len; i++) {
            print_math_tree (tree.parts[i]);
            printf ((i+1 != tree.tree_len) ? "+" : "");
        }
        printf (")");
    }
    else if (tree.type == EXPR_MULT) {
        printf ("(");
        for (int i=0; i < tree.tree_len; i++) {
            print_math_tree (tree.parts[i]);
            printf ((i+1 != tree.tree_len) ? "*" : "");
        }
        printf (")");
    }
    else if (tree.type == EXPR_CONST) {
        printf (tree.data > 0 ? "%.0f" : "(%.0f)", tree.data);
    }
    else if (tree.type == EXPR_VAR) {
        if (fabs (tree.data - 1) > 0.001)
            printf ("x^%.0f", tree.data);
        else
            printf ("x");
    }
}

void print_tree_data (math_tree tree) {
    printf(
        "%d %d %.0f\n",
        tree.type, tree.tree_len, tree.data
    );
    for (int i=0; i < tree.tree_len; i++)
        print_tree_data (tree.parts[i]);
}


// tree analysis functions

math_tree create_monomial (double num, double power) {
    math_tree tree = {
        .type = EXPR_MULT, .data = 0,
        .parts  = NULL, .tree_len = 0
    };
    // TODO float comparison
    if (power && num != 1) {
        tree.parts = calloc (2, sizeof (tree));
        tree.tree_len = 2;
        tree.parts[0] = (math_tree) {
            .type = EXPR_CONST, .parts = NULL,
            .data = num, .tree_len = 0
        };
        tree.parts[1] = (math_tree) {
            .type = EXPR_VAR, .parts = NULL,
            .data = power, .tree_len = 0
        };
    }
    else if (power)
        tree = (math_tree) {
            .type = EXPR_VAR, .parts = NULL,
            .data = power, .tree_len = 0
        };
    else
        tree = (math_tree) {
            .type = EXPR_CONST, .parts = NULL,
            .data = num, .tree_len = 0
        };
    return tree;
}

void simplify_tree (math_tree *tree) {
    for (int i=0; i < tree->tree_len; i++)
        simplify_tree (tree->parts + i);
    
    if (tree->type == EXPR_SUM || tree->type == EXPR_MULT) {
        // (x+1)+2 => x+1+2
        int new_len = tree->tree_len;
        for (int i=0; i < tree->tree_len; i++)
            if (tree->parts[i].type == tree->type)
                new_len += tree->parts[i].tree_len - 1;

        if (new_len != tree->tree_len) {
            tree->parts = realloc(
                tree->parts, sizeof (*tree) * new_len
            );

            // TODO make code more beautiful
            for (int i = 0, n = tree->tree_len; i < tree->tree_len; i++)
                if (tree->parts[i].type == tree->type) {
                    for (int k=1; k < tree->parts[i].tree_len; k++)
                        tree->parts[n++] = tree->parts[i].parts[k];
                    
                    void *t = tree->parts[i].parts;
                    tree->parts[i] = tree->parts[i].parts[0];
                    free (t);
                }

            tree->tree_len = new_len;
        }
    }
}

void free_math_tree (math_tree tree) {
    for (int i=0; i < tree.tree_len; i++)
        free_math_tree (tree.parts[i]);
    free (tree.parts);
}
