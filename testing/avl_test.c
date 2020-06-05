#include "../include/AVL.h"
#include "../include/stack.h"
#include "../include/test.h"

void test_inorder(struct AVLTree *tree);

int main() {
    struct AVLTree *tree = init_avl();
    assert(tree != NULL);

    test_inorder(tree);
    destroy_avl(tree);
    tree = NULL;
    assert(tree == NULL);
    return 0;
}

void test_inorder(struct AVLTree *tree) {
    int *vals = malloc(sizeof(int) * 100);
    int i = 0;
    for (i = 0; i < 100; i++) {
        vals[i] = i;
        insert(tree, &(vals[i]), i);
    }
    struct stack *stack = init_stack();
    inorder(tree, stack);
    int *ret = NULL;
    i = 99;
    while ((ret = pop(stack)) != NULL) {
        assert(*ret == vals[i]);
        --i;
    }
    //free(vals);
    destroy_stack(stack);
    stack = NULL;
    free(vals);
    fprintf(stderr, "AVL TREE PREORDER PASSED\n");
}