CC=gcc
CFLAGS=-Wall	-Werror	-g	-O2 
LDFLAGS= -lm -lpthread -L. -ltopologic

OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
SRC=src/topologic.c src/vertex.c src/edge.c src/stack.c src/topologic.c src/AVL.c src/topologic_json.c src/request.c
INCLUDES= include/AVL.h include/topologic.h include/vertex.h include/stack.h include/edge.h include/request.h

TESTS=testing/stack_test testing/avl_test #ADD MORE AS THEY GO
TEST_SRC=testing/stack_test.c testing/avl_test.c #ADD MORE IF NEED BE
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_DIR=testing

all: $(BIN) $(TESTS)

$(BIN): $(OBJ) $(INCLUDES)
	$(AR) rcs libtopologic.a $(OBJ)

$(TESTS): $(BIN) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ libtopologic.a $(TEST_DIR)/$(@F).o $(LDFLAGS)



all:$(BIN)
.PHONY : clean

clean:
	rm -f libtopologic.a
	rm -f $(OBJ) $(BIN)
	rm -f $(TESTS) $(TEST_OBJ)
