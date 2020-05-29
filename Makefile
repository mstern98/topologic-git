CC=gcc
CFLAGS=-Wall	-Werror	-g	#-O2
LDFLAGS=#-lm -lpthread -L. -lTopological

OBJ=$(SRC:.c=.o)
AR=ar

BIN=libTopological.a
SRC=graph.c	func.c
INCLUDES=header.h



$(BIN): $(OBJ) $(INCLUDES)
	$(AR) rcs libTopological.a $(OBJ)
all:$(BIN)
.PHONY : clean

clean:
	rm -f libTopological.a
	rm -f $(OBJ) $(BIN)
