#Copyright (c) 2011-2015 Hiroshi Tsubokawa
#See LICENSE and README

CC = g++
OPT = -O3
CFLAGS = $(OPT) -Wall -ansi -pedantic-errors
LDFLAGS = -lscene -lm
RM = rm -f

topdir      := ../..
target_dir  := bin
target_name := ply2mesh
files       := ply2mesh plyfile

incdir  := $(topdir)/src
libdir  := $(topdir)/lib
target  := $(topdir)/$(target_dir)/$(target_name)

#sources := $(addsuffix .cc, $(files))
#objects := $(addsuffix .o, $(files))
#depends := $(addsuffix .d, $(files))
sources := ply2mesh.cc plyfile.c
objects := ply2mesh.o  plyfile.o
depends := ply2mesh.d  plyfile.d

.PHONY: all clean depend
all: $(target)

plyfile.o: %.o: %.c
	@echo '  compile $<'
	@gcc   $(CFLAGS) -I$(incdir) -c -o $@ $<

ply2mesh.o: %.o: %.cc
	@echo '  compile $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -o $@ $<

$(target): $(objects)
	@echo '  link $(target_name)'
	@$(CC) -o $@ $^ -L$(libdir) $(LDFLAGS)

$(depends): %.d: %.cc
	@echo '  dependency $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -MM $< > $@

clean:
	@echo '  clean $(target_name)'
	@$(RM) $(target) $(objects) $(depends) 

ifneq "$(MAKECMDGOALS)" "clean"
-include $(depends)
endif

