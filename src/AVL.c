#include "../include/AVL.h"

struct AVLTree *init_avl()
{
    struct AVLTree *tree = (struct AVLTree*) malloc(sizeof(struct AVLTree));
    if (!tree)
        return NULL;

    tree->size = 0;
    tree->root = NULL;

    return tree;
}

int balance(struct AVLNode *node)
{
    if (!node)
        return 0;
    if (!node->right && !node->left)
        return 0;
    else if (!node->right)
        return node->left->height;
    else if (!node->left)
        return -node->right->height;
    return node->left->height - node->right->height;
}

int max_height(struct AVLNode *node)
{
    if (!node)
        return 0;
    if (!node->right && !node->left)
        return 0;
    if (!node->right)
        return node->left->height;
    if (!node->left)
        return node->right->height;
    if (node->right->height > node->left->height)
        return node->right->height;
    return node->left->height;
}

struct AVLNode *right_rotate(struct AVLNode *node)
{
    struct AVLNode *left = node->left;
    struct AVLNode *left_right = left->right;

    left->right = node;
    node->left = left_right;

    node->height = 1 + max_height(node);
    left->height = 1 + max_height(left);
    return left;
}

struct AVLNode *left_rotate(struct AVLNode *node)
{
    struct AVLNode *right = node->right;
    struct AVLNode *right_left = right->left;

    right->left = node;
    node->right = right_left;

    node->height = 1 + max_height(node);
    right->height = 1 + max_height(right);
    return right;
}

struct AVLNode *create_node(void *data, int id)
{
    struct AVLNode *node = (struct AVLNode*) malloc(sizeof(struct AVLNode));
    if (!node)
        return NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    node->id = id;
    node->data = data;
    return node;
}

struct AVLNode *insert_node(struct AVLNode *node, struct AVLNode *insert)
{
    int balance_factor = 0;
    if (!node)
    {
        return insert;
    }
    else if (insert->id < node->id)
        node->left = insert_node(node->left, insert);
    else
        node->right = insert_node(node->right, insert);
    node->height = 1 + max_height(node);
    balance_factor = balance(node);

    if (balance_factor > 1)
    {
        if (balance(node->left) >= 0)
            return right_rotate(node);
        else
        {
            node->left = left_rotate(node->left);
            return right_rotate(node);
        }
    }
    else if (balance_factor < -1)
    {
        if (balance(node->right) <= 0)
            return left_rotate(node);
        else
        {
            node->right = right_rotate(node->right);
            return left_rotate(node);
        }
    }
    return node;
}

int insert(struct AVLTree *tree, void *data, int id)
{
    if (!tree)
        return -1;
    struct AVLNode *node = create_node(data, id);
    if (!node)
        return -1;

    tree->root = insert_node(tree->root, node);
    tree->size++;

    return 0;
}

void *find_node(struct AVLNode *root, int id)
{
    if (!root)
        return NULL;
    if (id == root->id)
        return root->data;
    if (id < root->id)
        return find_node(root->left, id);
    else
        return find_node(root->right, id);
}

void *find(struct AVLTree *tree, int id)
{
    if (!tree)
        return NULL;
    return find_node(tree->root, id);
}

struct AVLNode *minNode(struct AVLNode *node)
{
    struct AVLNode *temp = node;
    while (temp->left != NULL)
    {
        temp = temp->left;
    }

    return temp;
}

struct AVLNode *remove_node(struct AVLNode *root, int id, void **data)
{
    int balance_factor = 0;
    if (!root)
        return root;
    else if (id < root->id)
        root->left = remove_node(root->left, id, data);
    else if (id > root->id)
        root->right = remove_node(root->right, id, data);
    else
    {
        if (data)
            *data = root->data;
        if (root->left == NULL || root->right == NULL)
        {
            struct AVLNode *temp = root->right ? root->right : root->left;
            if (!temp)
            {
                temp = root;
                root = NULL;
            }
            else
            {
                *root = *temp;
            }
            free(temp);
            temp = NULL;
        }
        else
        {
            struct AVLNode *temp = minNode(root->right);
            root->id = temp->id;
            root->data = temp->data;
            root->right = remove_node(root->right, temp->id, NULL);
        }
    }

    if (!root)
    {
        return root;
    }

    root->height = 1 + max_height(root);
    balance_factor = balance(root);

    if (balance_factor > 1)
    {
        if (balance(root->left) >= 0)
            return right_rotate(root);
        else
        {
            root->left = left_rotate(root->left);
            return right_rotate(root);
        }
    }
    else if (balance_factor < -1)
    {
        if (balance(root->right) <= 0)
            return left_rotate(root);
        else
        {
            root->right = right_rotate(root->right);
            return left_rotate(root);
        }
    }

    return root;
}

void *remove_ID(struct AVLTree *tree, int id)
{
    if (!tree || !tree->root)
        return NULL;
    void *data = NULL;
    tree->root = remove_node(tree->root, id, &data);
    if (data != NULL)
        tree->size--;
    return data;
}

void inorder_nodes(struct AVLNode *node, struct stack *stack)
{
    if (!node || node->id == -1)
        return;
    inorder_nodes(node->left, stack);
    push(stack, node->data);
    inorder_nodes(node->right, stack);
}

void inorder(struct AVLTree *tree, struct stack *stack)
{
    if (!tree || !stack)
        return;
    if (tree->size <= 0)
        return;
    inorder_nodes(tree->root, stack);
}

void preorder_nodes(struct AVLNode *node, struct stack *stack)
{
    if (!node || node->id == -1)
        return;
    push(stack, node->data);
    preorder_nodes(node->left, stack);
    preorder_nodes(node->right, stack);
}

void preorder(struct AVLTree *tree, struct stack *stack)
{
    if (!tree || !stack)
        return;
    if (tree->size <= 0)
        return;
    preorder_nodes(tree->root, stack);
}

void postorder_nodes(struct AVLNode *node, struct stack *stack)
{
    if (!node || node->id == -1)
        return;
    postorder_nodes(node->left, stack);
    postorder_nodes(node->right, stack);
    push(stack, node->data);
}

void postorder(struct AVLTree *tree, struct stack *stack)
{
    if (!tree || !stack)
        return;
    if (tree->size <= 0)
        return;
    postorder_nodes(tree->root, stack);
}

void stackify_nodes(struct AVLNode *node, struct stack *stack)
{
    if (!node)
        return;
    stackify_nodes(node->left, stack);
    push(stack, node->data);
    stackify_nodes(node->right, stack);

    node->data = NULL;
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    free(node);
    node = NULL;
}

void stackify(struct AVLTree *tree, struct stack *stack)
{
    if (!tree || !stack)
        return;
    stackify_nodes(tree->root, stack);

    tree->root = NULL;
    tree->size = 0;
    free(tree);
    tree = NULL;
}

void destroy_avl_nodes(struct AVLNode *node)
{
    if (!node)
        return;
    node->data = NULL;
    node->height = 0;
    if (node->left)
        destroy_avl_nodes(node->left);
    if (node->right)
        destroy_avl_nodes(node->right);
    node->left = NULL;
    node->right = NULL;
    free(node);
    node = NULL;
}

void destroy_avl(struct AVLTree *tree)
{
    if (!tree)
        return;
    destroy_avl_nodes(tree->root);

    tree->root = NULL;
    tree->size = 0;
    free(tree);
    tree = NULL;
}
