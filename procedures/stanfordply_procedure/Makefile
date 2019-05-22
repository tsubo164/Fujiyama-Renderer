#Copyright (c) 2011-2017 Hiroshi Tsubokawa
#See LICENSE and README

CC = g++
OPT = -O3
CFLAGS = $(OPT) -fPIC -Wall -std=c++11 -pedantic-errors
LDFLAGS = -shared -lscene
RM = rm -f

topdir      := ../..
target_dir  := lib
target_name := StanfordPlyProcedure.so
files       := stanfordply_procedure ply2mesh plyfile

incdir  := $(topdir)/src
libdir  := $(topdir)/lib
target  := $(topdir)/$(target_dir)/$(target_name)

#sources := $(addsuffix .cc, $(files))
sources := stanfordply_procedure.cc ply2mesh.cc plyfile.c
objects := $(addsuffix .o, $(files))
depends := $(addsuffix .d, $(files))

.PHONY: all clean depend
all: $(target)

#compling with gcc instead of g++ because of lots of errors and warnings
#disabled -Wself-assign
plyfile.o: %.o: %.c
	@echo '  compile $<'
	@gcc   $(OPT) -fPIC -Wall -Wno-self-assign -ansi -pedantic-errors -I$(incdir) -c -o $@ $<


ply2mesh.o: %.o: %.cc
	@echo '  compile $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -o $@ $<

stanfordply_procedure.o: %.o: %.cc
	@echo '  compile $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -o $@ $<

#$(objects): %.o: %.cc
#	@echo '  compile $<'
#	@$(CC) $(CFLAGS) -I$(incdir) -c -o $@ $<

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
