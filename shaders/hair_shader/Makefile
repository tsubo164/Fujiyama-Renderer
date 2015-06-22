#Copyright (c) 2011-2015 Hiroshi Tsubokawa
#See LICENSE and README

CC = g++
OPT = -O3
CFLAGS = $(OPT) -fPIC -Wall -ansi -pedantic-errors
LDFLAGS = -shared -lscene
RM = rm -f

topdir      := ../..
target_dir  := lib
target_name := HairShader.so
files       := hair_shader

incdir  := $(topdir)/src
libdir  := $(topdir)/lib
target  := $(topdir)/$(target_dir)/$(target_name)

sources := $(addsuffix .cc, $(files))
objects := $(addsuffix .o, $(files))
depends := $(addsuffix .d, $(files))

.PHONY: all clean depend
all: $(target)

$(objects): %.o: %.cc
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

