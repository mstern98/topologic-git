CC=gcc
CFLAGS=-Wall	-Werror	-g	-O2 
LDFLAGS=#-lm -lpthread -L. -lTopological

OBJ=$(SRC:.c=.o)
AR=ar

BIN=libTopological.a
SRC=src/topologic.c src/vertex.c src/edge.c src/stack.c src/topologic.c src/AVL.c
INCLUDES= include/AVL.h include/topologic.h include/vertex.h include/stack.h include/edge.h

TEST1=testing/Test1 #ADD MORE AS THEY GO
TEST_SRC=testing/Test1.c #ADD MORE IF NEED BE
TEST_OBJ=$(TEST_SRC:.c=.o)

$(BIN): $(OBJ) $(INCLUDES)
	$(AR) rcs libtopologic.a $(OBJ)





all:$(BIN)
.PHONY : clean

clean:
	rm -f libtopologic.a
	rm -f $(OBJ) $(BIN)
	rm -f $(TESTS) $(TEST_OBJ)
