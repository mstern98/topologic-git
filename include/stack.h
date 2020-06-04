#ifndef __TOPOLOGIC_STACK__
#define __TOPOLOGIC_STACK__
//#include "./topologic.h"
#include <stdio.h>


struct stack_node {
    void *data;
    struct stack_node *next;
};

struct stack{
    struct stack_node *root;
    int length;
};

struct stack *init_stack();
void *get(struct stack *stack, int index);
void *pop(struct stack *stack);
int push(struct stack *stack, void *data);
void destroy_stack(struct stack *stack);

#endif
