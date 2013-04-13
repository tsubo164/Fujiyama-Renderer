#Copyright (c) 2011-2013 Hiroshi Tsubokawa
#See LICENSE and README

topdir := ../

CC = gcc
OPT = -O3
CFLAGS = -I$(topdir)src -Wall -ansi -pedantic-errors $(OPT)
LDFLAGS = -L$(topdir)lib -lscene

RM = rm -f

.PHONY: all check clean
all: check

files := Array Box Numeric Vector
objects := $(addsuffix _test.o, $(files))
targets := $(addsuffix _test, $(files))

Test.o: Test.c Test.h
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo '  compile' $<

$(objects) : %.o : %.c
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo '  compile' $<

$(targets) : % : %.o Test.o
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo '  build' $@

check: $(targets)
	@for t in $^; \
	do echo running :$$t; env LD_LIBRARY_PATH=$(topdir)lib ./$$t; \
	done;

clean:
	-$(RM) Test.o $(objects)
	-$(RM) $(targets)

