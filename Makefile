CC=gcc
CFLAGS=-Wall	-Werror	-g	-O2 
LDFLAGS= -lm -lpthread -L. -ltopologic

OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
SRC=src/topologic.c src/vertex.c src/edge.c src/stack.c src/topologic.c src/AVL.c
INCLUDES= include/AVL.h include/topologic.h include/vertex.h include/stack.h include/edge.h

TESTS=testing/Test1 testing/StackTest #ADD MORE AS THEY GO
TEST_SRC=testing/test1.c testing/stack_test.c /#ADD MORE IF NEED BE
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_DIR=testing

all: $(BIN) Test

$(BIN): $(OBJ) $(INCLUDES)
	$(AR) rcs libtopologic.a $(OBJ)

Test: $(BIN) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(TEST_DIR)/StackTest libtopologic.a $(TEST_DIR)/stack_test.o $(LDFLAGS)



all:$(BIN)
.PHONY : clean

clean:
	rm -f libtopologic.a
	rm -f $(OBJ) $(BIN)
	rm -f $(TESTS) $(TEST_OBJ)
