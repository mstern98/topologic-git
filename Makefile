CC=gcc

LDFLAGS= -lm -lpthread -L. -ltopologic -pthread -lfl
CFLAGS=-Wall	-Werror	-g	#-O2
OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
<<<<<<< HEAD
SRC=$(wildcard src/*.c) $(wildcard parse/*.c)
INCLUDES= $(wildcard include/*.h) $(wildcard parse/*.h)

FLEX=parse/topologic_parser.lex
BISON=parse/topologic_parser.y
FLEX_C=$(FLEX:.lex=.yy.c)
BISON_C=$(BISON:.y=.tab.c)
FLEX_OBJ=$(FLEX_C:.c=.o)
BISON_OBJ=$(BISON_C:.c=.o)

TESTS=$(TEST_SRC:.c=)#ADD MORE AS THEY GO
TEST_SRC=$(wildcard testing/*.c)  #ADD MORE IF NEED BE
=======
SRC=src/topologic.c src/vertex.c src/edge.c src/stack.c src/topologic.c src/AVL.c src/topologic_json.c src/request.c src/graph.c
INCLUDES= include/AVL.h include/topologic.h include/vertex.h include/stack.h include/edge.h include/request.h include/graph.h

TESTS=testing/stack_test testing/avl_test testing/graph_vertex_edge_test testing/mem_option_test #ADD MORE AS THEY GO
TEST_SRC=testing/stack_test.c testing/avl_test.c testing/graph_vertex_edge_test.c testing/mem_option_test.c #ADD MORE IF NEED BE
>>>>>>> 466308024f77e16d4f29ba63daf06d3dbf6f320a
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_DIR=testing

all: $(FLEX) $(BISON) $(FLEX_C) $(BISON_C) $(BIN) $(TESTS)

$(FLEX_C):
	flex $(FLEX)
	mv lex.yy.c $(FLEX_C)
	$(CC) -g -c $(FLEX_C) -o $(FLEX_OBJ)
$(BISON_C): $(BISON)
	bison -d $(BISON) -o $(BISON_C)
	$(CC) -g -c $(BISON_C) -o $(BISON_OBJ)

$(BIN): $(OBJ) $(INCLUDES)
	$(AR) rcs libtopologic.a $(OBJ) 

$(TESTS): $(BIN) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ libtopologic.a $(TEST_DIR)/$(@F).o $(LDFLAGS)



all:$(BIN)
.PHONY : clean

clean:
	rm -f libtopologic.a
	rm -f $(FLEX_C) $(FLEX_OBJ)
	rm -f $(BISON_C) $(BISON_OBJ)
	rm -f $(OBJ) $(BIN)
	rm -f $(TESTS) $(TEST_OBJ)
