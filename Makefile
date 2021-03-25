# SPDX-License-Identifier: MIT WITH bison-exception WITH swig-exception
# Copyright © 2020 Matthew Stern, Benjamin Michalowicz

CC=gcc
MCS=$(MCS)

LDFLAGS= -lm -lpthread -L. -ltopologic -pthread #-lfl
CFLAGS=-Wall	-Werror	-g	-fPIC -O2 #-fsanitize=thread 
OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
SRC=$(wildcard src/*.c)
INCLUDES= $(wildcard include/*.h)

TESTS=$(TEST_SRC:.c=)#ADD MORE AS THEY GO
TEST_SRC=$(wildcard testing/*.c)  #ADD MORE IF NEED BE
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
	rm -f $(TEST_OBJ) $(TESTS)
	rm -f $(OBJ) $(BIN)
