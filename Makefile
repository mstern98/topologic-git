CC=gcc

LDFLAGS= -lm -lpthread -L. -ltopologic -pthread -lfl
CFLAGS=-Wall	-Werror	-g	#-O2
OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
SRC=$(wildcard src/*.c)
INCLUDES= $(wildcard include/*.h)

FLEX=parse/topologic_parser.lex
BISON=parse/topologic_parser.y
FLEX_C=$(FLEX:.lex=.yy.c)
BISON_C=$(BISON:.y=.tab.c)
FLEX_OBJ=$(FLEX_C:.c=.o)
BISON_OBJ=$(BISON_C:.c=.o)

TESTS=$(TEST_SRC:.c=)#ADD MORE AS THEY GO
TEST_SRC=$(wildcard testing/*.c)  #ADD MORE IF NEED BE
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

$(BIN): $(OBJ) $(INCLUDES) $(BISON_OBJ) $(FLEX_OBJ)
	$(AR) rcs libtopologic.a $(OBJ) $(BISON_OBJ) $(FLEX_OBJ)

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
