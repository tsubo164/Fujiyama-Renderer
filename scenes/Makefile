#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE and README

topdir := ../

CC = gcc
OPT = -O3
CFLAGS = -I$(topdir)src -Wall -ansi -pedantic-errors $(OPT)
LDFLAGS = -L$(topdir)lib -lscene

RM = rm -f

.PHONY: all help sample check clean cube.fb
all: help

help:
	@echo
	@echo 'Run this command to render sample scene file written in c'
	@echo '  $$ make sample'
	@echo

sample: cube.fb $(topdir)bin/fbview
	env LD_LIBRARY_PATH=$(topdir)lib $(topdir)bin/fbview $<

cube.fb: cube cube.mesh
	env LD_LIBRARY_PATH=$(topdir)lib ./cube

cube.mesh: cube.ply $(topdir)bin/ply2mesh
	env LD_LIBRARY_PATH=$(topdir)lib $(topdir)bin/ply2mesh $< $@

cube: cube.c $(topdir)lib/libscene.so $(topdir)lib/PlasticShader.so
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@echo '  clean samples'
	@-$(RM) cube cube.fb cube.mesh
