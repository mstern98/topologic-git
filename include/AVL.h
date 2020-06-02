#ifndef __TOPOLOGIC_AVL__
#define __TOPOLOGIC_AVL__

struct AVLNode {
    void *data;
    int id;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
};

struct AVLTree{
    struct AVLNode *root;
    int height;
};

struct AVLTree *init_avl();
int insert(struct AVLTree *tree, void *data, int id);
void *remove(struct AVLTree *tree, int id);
void *find(struct AVLTree *tree, int id);

#endif