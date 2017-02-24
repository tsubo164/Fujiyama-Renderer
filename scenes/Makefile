#Copyright (c) 2011-2017 Hiroshi Tsubokawa
#See LICENSE and README

topdir := ../

CC = g++
OPT = -O3
CFLAGS = -I$(topdir)src -Wall -ansi -pedantic-errors $(OPT)
LDFLAGS = -L$(topdir)lib -lscene

RM = rm -f

ifeq ($(shell uname),Darwin)
ld_path_name = DYLD_LIBRARY_PATH
else
ld_path_name = LD_LIBRARY_PATH
endif

.PHONY: all help sample check clean cube.fb
all: help

help:
	@echo
	@echo 'Run this command to render sample scene file written in c'
	@echo '  $$ make sample'
	@echo

sample: cube.fb $(topdir)bin/fbview
	env $(ld_path_name)=$(topdir)lib $(topdir)bin/fbview $<

cube.fb: cube cube.mesh
	env $(ld_path_name)=$(topdir)lib ./cube

cube.mesh: cube.ply $(topdir)bin/ply2mesh
	env $(ld_path_name)=$(topdir)lib $(topdir)bin/ply2mesh $< $@

cube: cube.cc $(topdir)lib/libscene.so $(topdir)lib/PlasticShader.so
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	@echo '  clean samples'
	@-$(RM) cube cube.fb cube.mesh
