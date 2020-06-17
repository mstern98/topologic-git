#ifndef __TOPOLOGIC_AVL__
#define __TOPOLOGIC_AVL__

#include <stdio.h>
#include <stdlib.h>
#include "./stack.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AVLNode
{
    void *data;
    int id;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
};

struct AVLTree
{
    struct AVLNode *root;
    int size;
};

struct AVLTree *init_avl();
int insert(struct AVLTree *tree, void *data, int id);
void *remove_ID(struct AVLTree *tree, int id);
void *find(struct AVLTree *tree, int id);
void inorder(struct AVLTree *tree, struct stack *stack); //in order
void postorder(struct AVLTree *tree, struct stack *stack);
void preorder(struct AVLTree *tree, struct stack *stack);
void stackify(struct AVLTree *tree, struct stack *stack); //same as get_nodes but destroys tree
void destroy_avl(struct AVLTree *tree);

#ifdef __cplusplus
}
#endif

#endif
