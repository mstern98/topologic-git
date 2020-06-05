#include "../include/stack.h"
#include "../include/test.h"

#define VALUE 10

void test_push(struct stack *stack);
void test_pop(struct stack *stack);
void test_pop_all(struct stack *stack);

int main() {
    fprintf(stderr, "TESTING STACK...\n");
    struct stack *stack = init_stack();
    assert(stack != NULL);

    test_push(stack);
    test_pop(stack);
    test_pop_all(stack);

    destroy_stack(stack);
    stack = NULL;
    assert(stack == NULL);
    fprintf(stderr, "STACK PASSED\n");
    return 0;
}

void test_push(struct stack *stack) {
    int *value = malloc(sizeof(int));
    *value = VALUE;
    assert(push(stack, value) == 0);
    fprintf(stderr, "STACK PUSH PASSED\n");
}

void test_pop(struct stack *stack) {
    int *value = (int *) pop(stack);

    assert(VALUE == *value);
    assert(NULL == pop(stack));
    free(value);
    fprintf(stderr, "STACK POP PASSED\n");
}

void test_pop_all(struct stack *stack) {
    int **data = malloc(sizeof(int *) * 6);
    
    int length = 6;
    int i = 0;

    for (i = 0; i < length; i++) {
        data[i] = malloc(sizeof(int));
        *(data[i]) = i;
    }
    for (i = length - 1; i >= 0; i--) {
        assert(push(stack, data[i]) == 0);
    }
    for (i = 0; i < length; i++) {
        int *val = ((int *) pop(stack));
        assert(*val == *(data[i]));
        free(val);
    }
    free(data);
    fprintf(stderr, "STACK PUSHED AND POPPED A LOT PASSED\n");
}
