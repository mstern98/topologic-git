/* SPDX-License-Identifier: MIT WITH bison-exception */
/* Copyright © 2020 Matthew Stern, Benjamin Michalowicz */

#ifndef __TOPOLOGIC_STACK__
#define __TOPOLOGIC_STACK__

#include <stdio.h>
#include <stdlib.h>

struct stack_node
{
    void *data;
    struct stack_node *next;
};

struct stack
{
    struct stack_node *root;
    int length;
};

struct stack *init_stack();
void *get(struct stack *stack, int index);
void *pop(struct stack *stack);
int push(struct stack *stack, void *data);
void destroy_stack(struct stack *stack);

#endif
