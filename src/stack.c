#include "../include/stack.h"

struct stack *init_stack()
{
    struct stack *stack = (struct stack*) malloc(sizeof(struct stack));
    if (!stack)
        return NULL;
    stack->root = NULL;
    stack->length = 0;
    return stack;
}

void *pop(struct stack *stack)
{
    if (!stack || stack->length <= 0)
        return NULL;

    struct stack_node *node = stack->root;
    void *data = node->data;
    stack->root = node->next;
    stack->length--;
    node->next = NULL;
    node->data = NULL;
    free(node);
    node = NULL;

    return data;
}

void *get(struct stack *stack, int index)
{
    if (!stack || stack->length == 0)
    if (index < 0 || !stack || index >= stack->length)
        return NULL;

    int i = 0;
    struct stack_node *node = stack->root;
    for (i = 0; i < index; i++)
        node = node->next;

    return node->data;
}

int push(struct stack *stack, void *data)
{
    if (!stack)
        return -1;

    struct stack_node *node = (struct stack_node*) malloc(sizeof(struct stack_node));
    if (!node)
        return -1;

    node->data = data;
    node->next = stack->root;
    stack->root = node;
    stack->length++;
    return 0;
}

void destroy_stack(struct stack *stack)
{
    if (!stack)
        return;
    struct stack_node *node = stack->root;
    struct stack_node *prev = NULL;
    while (node != NULL)
    {
        prev = node;
        node = node->next;
        prev->next = NULL;
        prev->data = NULL;
        free(prev);
        prev = NULL;
    }
    stack->length = 0;
    free(stack);
    stack = NULL;
}
