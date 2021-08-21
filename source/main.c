#include <stdio.h>

#include "math_tree.h"

void test_expr (char *str);


int main (void) {
    char *inputs[] = {
        "x+2", "3*x^2-2*x+3","(x+2)+1",
        "152*x*3-x^3*x^2*3",
        "(x-2)*(x-3)*(2*x^2-3)-37*(x^2-3*x^5)",
        "(37*x^3-42*(43-8)+x+32*x)*(657*(657*879*x^2))+x^3*3"
    };
    for (int i=0; i < sizeof (inputs) / sizeof (char*); i++)
        test_expr (inputs[i]);
    return 0;
}


void test_expr (char *str) {
    math_tree tree = read_sum (str, NULL);
    print_math_tree (tree);
    printf ("\n");

    simplify_tree (&tree);
    print_math_tree (tree);
    printf ("\n\n");
    
    free_math_tree (tree);
}