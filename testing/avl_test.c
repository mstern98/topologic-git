#include "../include/AVL.h"
#include "../include/stack.h"
#include "../include/test.h"

void test_insert(struct AVLTree *tree, int *vals);
void test_inorder(struct AVLTree *tree);
void test_find(struct AVLTree *tree);
void test_delete(struct AVLTree *tree);

int main() {
    struct AVLTree *tree = init_avl();
    assert(tree != NULL);

    int *vals = malloc(sizeof(int) * 100);
    
    test_insert(tree, vals);
    test_inorder(tree);
    test_find(tree);
    test_delete(tree);
    destroy_avl(tree);
    tree = NULL;
    assert(tree == NULL);
    free(vals);
    return 0;
}

void test_insert(struct AVLTree *tree, int *vals) {
    int i = 0;
    for (i = 0; i < 100; i++) {
        vals[i] = i;
        assert(insert(tree, &(vals[i]), i) == 0);
    }
    fprintf(stderr, "AVL TREE INSERT PASSED");
}

void test_inorder(struct AVLTree *tree) {
    
    struct stack *stack = init_stack();
    inorder(tree, stack);
    int *ret = NULL;
    int i = 99;
    while ((ret = pop(stack)) != NULL) {
        assert(*ret == i);
        --i;
    }
    destroy_stack(stack);
    stack = NULL;
    fprintf(stderr, "AVL TREE PREORDER PASSED\n");
}

void test_find(struct AVLTree *tree) {
    assert(*((int *) find(tree, 10)) == 10);
    assert(*((int *) find(tree, 23)) == 23);
    assert(*((int *) find(tree, 99)) == 99);
    assert(*((int *) find(tree, 0)) == 0);
    assert(find(tree, -1) == NULL);
    assert(find(tree, 100) == NULL);
    fprintf(stderr, "AVL TREE FIND PASSED\n");
}

void test_delete(struct AVLTree *tree) {
    assert(*((int *) remove_ID(tree, 10)) == 10);
    assert(*((int *) remove_ID(tree, 23)) == 23);
    assert(*((int *) remove_ID(tree, 99)) == 99);
    assert(*((int *) remove_ID(tree, 0)) == 0);
    assert(find(tree, 10) == NULL);
    assert(find(tree, 23) == NULL);
    assert(find(tree, 99) == NULL);
    assert(find(tree, 0) == NULL);
    assert(remove_ID(tree, -1) == NULL);
    assert(remove_ID(tree, 100) == NULL);
    int *i = NULL;
    struct stack *stack = init_stack();
    inorder(tree, stack);
    while ((i = (int *) pop(stack)) != NULL) {
        assert(*i != 10);
        assert(*i != 23);
        assert(*i != 99);
        assert(*i != 0);
    }
    destroy_stack(stack);
    fprintf(stderr, "AVL TREE DELETE PASSED\n");
}