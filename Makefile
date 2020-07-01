# SPDX-License-Identifier: MIT WITH bison-exception WITH swig-exception
# Copyright © 2020 Matthew Stern, Benjamin Michalowicz

CC=gcc
CXX=g++
MCS=$(MCS)

LDFLAGS= -lm -lpthread -L. -ltopologic -pthread -lfl
CFLAGS=-Wall	-Werror	-g	-fPIC #-O2
OBJ=$(SRC:.c=.o)
AR=ar

BIN=topologic
SRC=$(wildcard src/*.c)
INCLUDES= $(wildcard include/*.h)

FLEX=parse/topologic_parser.lex
BISON=parse/topologic_parser.y
FLEX_C=$(FLEX:.lex=.yy.c)
BISON_C=$(BISON:.y=.tab.c)
BISON_H=$(BISON:.y=.tab.h)
FLEX_OBJ=$(FLEX_C:.c=.o)
BISON_OBJ=$(BISON_C:.c=.o)

FLEXPP=parse/topologic_parser_cpp.lex
BISONPP=parse/topologic_parser_cpp.ypp
FLEX_CPP=$(FLEXPP:.lex=.yy.cpp)
BISON_CPP=$(BISONPP:.ypp=.tab.cpp)
BISON_HPP=$(BISONPP:.ypp=.tab.hpp)
FLEX_OBJ_PP=$(FLEX_CPP:.cpp=.o)
BISON_OBJ_PP=$(BISON_CPP:.cpp=.o)

TOPYLOGIC_I=topylogic/topylogic.i
TOPYLOGIC_WRAP=topylogic/topylogic_wrap.c
TOPYLOGIC_SO=topylogic/_topylogic.so
TOPYLOGIC_PY=topylogic/topylogic.py
TOPYLOGIC_O=$(wildcard topylogic/*.o)

CSHARP_I=topologicsharp/topologicsharp.i
CSHARP_WRAP=topologicsharp/topologicsharp_wrap.c
CSHARP_SO=topologicsharp/_topologicsharp.so
CSHARP_CS=topologicsharp/topologicsharp.cs
CSHARP_O=$(wildcard topologicsharp/*.o)

TESTS=$(TEST_SRC:.c=)#ADD MORE AS THEY GO
TEST_SRC=$(wildcard testing/*.c)  #ADD MORE IF NEED BE
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_DIR=testing

all: $(BISON) $(BISON_C) $(BISON_H) $(FLEX) $(FLEX_C) $(BIN) $(TESTS) 

$(FLEX_C):
	flex $(FLEX)
	mv lex.yy.c $(FLEX_C)
	$(CC) -fPIC -g -c $(FLEX_C) -o $(FLEX_OBJ)
$(BISON_C): $(BISON)
	bison -d $(BISON) -o $(BISON_C)
	$(CC) -fPIC -g -c $(BISON_C) -o $(BISON_OBJ)

$(BIN): $(OBJ) $(INCLUDES) $(BISON_OBJ) $(FLEX_OBJ)
	$(AR) rcs libtopologic.a $(OBJ) $(BISON_OBJ) $(FLEX_OBJ)

$(TESTS): $(BIN) $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ libtopologic.a $(TEST_DIR)/$(@F).o $(LDFLAGS)

python: $(OBJ) $(INCLUDES)
	swig -python $(TOPYLOGIC_I)
	$(CC) -c -fPIC topylogic/topylogic_wrap.c -o topylogic/topylogic_wrap.o -I/usr/include/python3.6m
	$(CC) -shared topylogic/topylogic_wrap.o $(OBJ) -o $(TOPYLOGIC_SO)

python2: $(OBJ) $(INCLUDES)
	swig -python $(TOPYLOGIC_I)
	$(CC) -c -fPIC topylogic/topylogic_wrap.c -o topylogic/topylogic_wrap.o -I/usr/include/python2.7
	$(CC) -shared topylogic/topylogic_wrap.o $(OBJ) -o $(TOPYLOGIC_SO)

csharp: $(OBJ) $(INCLUDES)
	swig -outfile topologicsharp.cs -csharp  $(CSHARP_I) 
	$(CC) -c -fPIC $(CSHARP_WRAP) -o topologicsharp/topologicsharp.o 
	@bash topologicsharp/make_dll.sh
	

cpp: $(BISON_CPP) $(BISON_OBJ_PP) $(BISON_HPP) $(FLEX_CPP) $(FLEX_OBJ_PP) $(OBJ) $(INCLUDES) 
	$(AR) rcs libtopologic.a $(OBJ) $(BISON_OBJ_PP) $(FLEX_OBJ_PP)

rust: 
	@bash rustopologic/rustCreation.sh

$(FLEX_CPP):
	flex $(FLEXPP)
	mv lex.yy.cc $(FLEX_CPP)
	$(CXX) -fPIC -g -c $(FLEX_CPP) -o $(FLEX_OBJ_PP)
$(BISON_CPP): $(BISONPP)
	bison -d $(BISONPP) -o $(BISON_CPP)
	$(CXX) -fPIC -g -c $(BISON_CPP) -o $(BISON_OBJ_PP)

all:$(BIN)
.PHONY : clean cpp python python2 rust csharp

clean:
	rm -f libtopologic.a
	rm -f $(FLEX_C) $(FLEX_OBJ)
	rm -f $(BISON_C) $(BISON_OBJ) $(BISON_H)
	rm -f $(FLEX_CPP) $(FLEX_OBJ_PP)
	rm -f $(BISON_CPP) $(BISON_OBJ_PP) $(BISON_HPP)
	rm -f $(OBJ) $(BIN)
	rm -f $(TOPYLOGIC_WRAP) $(TOPYLOGIC_PY) $(TOPYLOGIC_SO) $(TOPYLOGIC_O)
	rm -f $(CSHARP_WRAP) $(CSHARP_CS) $(CSHARP_SO) $(CSHARP_O)
	rm -f topologicsharp/*.cs
	rm -rf topylogic/__pycache__
	rm -rf topylogic/build
	-rm -f state_*
	-rm -f topylogic/state_*
	rm -f topologicsharp/*.dll
	rm -f $(TESTS) $(TEST_OBJ)
	-rm -f testing/*.exe
	rm -f rustopologic/RustTopologic/src/bindings.rs
	rm -f rustopologic/RustTopologic/src/*.c
	rm -f rustopologic/RustTopologic/Cargo.lock
	rm -f rustopologic/RustTopologic/rustTests/bindings.rs
	cd rustopologic/RustTopologic/ && cargo clean && rm -rf include/ && rm -f libtopologic.a
