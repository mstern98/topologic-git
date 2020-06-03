#ifndef __TOPOLOGIC_AVL__
#define __TOPOLOGIC_AVL__

#include <stdio.h>
#include "./stack.h"

struct AVLNode {
    void *data;
    int id;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
};

struct AVLTree{
    struct AVLNode *root;
    int size;
};

struct AVLTree *init_avl();
int insert(struct AVLTree *tree, void **data, int id);
void *remove_ID(struct AVLTree *tree, int id);
void *find(struct AVLTree *tree, int id);
void preorder(struct AVLTree *tree, struct stack *stack); //pre order
void stackify(struct AVLTree *tree, struct stack *stack); //same as get_nodes but destroys tree
void destroy_avl(struct AVLTree *tree);

#endif
