# Copyright (c) 2011-2019 Hiroshi Tsubokawa
# See LICENSE and README

topdir := ../

CC = g++
OPT = -O3
CFLAGS = -I$(topdir)src -Wall -std=c++11 -pedantic-errors $(OPT)
LDFLAGS = -L$(topdir)lib -lscene

RM = rm -f

.PHONY: all check clean
all: check

files := box numeric vector
objects := $(addsuffix _test.o, $(files))
targets := $(addsuffix _test, $(files))

unit_test.o: unit_test.cc unit_test.h
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo '  compile' $<

$(objects) : %.o : %.cc unit_test.h
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo '  compile' $<

$(targets) : % : %.o unit_test.o
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo '  build' $@

check: $(targets)
	@for t in $^; \
	do echo running :$$t; env LD_LIBRARY_PATH=$(topdir)lib ./$$t; \
	done;

clean:
	@echo '  clean tests'
	@-$(RM) unit_test.o $(objects)
	@-$(RM) $(targets)
	@-$(RM) *.bin
