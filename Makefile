#Copyright (c) 2011-2012 Hiroshi Tsubokawa
#See LICENSE and README

CC = gcc
CFLAGS = -Isrc -Wall -ansi -pedantic-errors -O2
#CFLAGS = -Isrc -Wall -ansi -pedantic-errors -O2 -g
#CFLAGS = -Isrc -Wall -ansi -pedantic-errors -pg
LDFLAGS = -Llib
#LDFLAGS = -Llib -g
#LDFLAGS = -Llib -pg
CPPFLAGS = -Isrc -Wall -O2

.PHONY: all all_ clean
all: all_

#******************************************************************************
# $(eval $(call submodule, $(srcdir_), $(files_), $(subtgt_), $(cflags_), $(ldflags_)))
define submodule
target := $$(addprefix $(tgtdir_)/, $(subtgt_) )
sources := $$(addprefix $(srcdir_)/, $$(addsuffix .c, $(files_)) )
objects := $$(addprefix $(srcdir_)/, $$(addsuffix .o, $(files_)) )
depends := $$(addprefix $(srcdir_)/, $$(addsuffix .d, $(files_)) )
all_targets += $$(target)
all_objects += $$(objects)
all_depends += $$(depends)
$$(objects): %.o: %.c
	$$(CC) $$(CFLAGS) $(cflags_) -c -o $$@ $$<
$$(target): $$(objects) $(addobj_)
	$$(CC) $$(LDFLAGS) $(ldflags_) -o $$@ $$^
$$(depends): %.d: %.c
	$$(CC) $$(CFLAGS) $(cflags_) -c -MM $$< | \
	sed 's,\($$(notdir $$*)\.o\) *:,$$(dir $$@)\1 $@: ,' > $$@.tmp
	mv $$@.tmp $$@
endef
#******************************************************************************
#******************************************************************************
# $(eval $(call submodule_cpp, $(srcdir_), $(files_), $(subtgt_), $(cflags_), $(ldflags_)))
define submodule_cpp
target := $$(addprefix $(tgtdir_)/, $(subtgt_) )
sources := $$(addprefix $(srcdir_)/, $$(addsuffix .c, $(files_)) )
objects := $$(addprefix $(srcdir_)/, $$(addsuffix .o, $(files_)) )
depends := $$(addprefix $(srcdir_)/, $$(addsuffix .d, $(files_)) )
all_targets += $$(target)
all_objects += $$(objects)
all_depends += $$(depends)
$$(objects): %.o: %.cpp
	g++ $$(CPPFLAGS) $(cflags_) -c -o $$@ $$<
$$(target): $$(objects) $(addobj_)
	g++ $$(LDFLAGS) $(ldflags_) -o $$@ $$^
$$(depends): %.d: %.cpp
	g++ $$(CPPFLAGS) $(cflags_) -c -MM $$< | \
	sed 's,\($$(notdir $$*)\.o\) *:,$$(dir $$@)\1 $@: ,' > $$@.tmp
	mv $$@.tmp $$@
endef
#******************************************************************************

all_targets :=
all_objects :=
all_depends :=
addobj_ :=

#core library
srcdir_  := src
tgtdir_  := lib
files_   := OS Array Box Matrix Transform Triangle Timer Plugin Property Progress \
	FrameBuffer FrameBufferIO Camera Mesh MeshIO Tiler Renderer Light Filter Mipmap\
	ObjectInstance Accelerator Sampler Shader SL Texture Scene SceneInterfaces 
subtgt_  := libscene.so
cflags_  := -fPIC
ldflags_ := -shared -ldl -lm
$(eval $(call submodule))

#shaders
srcdir_  := shaders/PlasticShader
tgtdir_  := lib
files_   := PlasticShader
subtgt_  := PlasticShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

srcdir_  := shaders/GlassShader
tgtdir_  := lib
files_   := GlassShader
subtgt_  := GlassShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

srcdir_  := shaders/ConstantShader
tgtdir_  := lib
files_   := ConstantShader
subtgt_  := ConstantShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

#tools
srcdir_  := tools/SceneParser
tgtdir_  := $(srcdir_)
files_   := main Parser Table
subtgt_  := scn
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

srcdir_  := tools/FrameBufferViewer
tgtdir_  := $(srcdir_)
files_   := main FrameBufferViewer
subtgt_  := fbview
cflags_  :=
ldflags_ := -lscene -lGL -lGLU -lglut
$(eval $(call submodule))

srcdir_  := tools/ply2mesh
tgtdir_  := $(srcdir_)
files_   := main plyfile
subtgt_  := ply2mesh
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

srcdir_  := tools/hdr2mip
tgtdir_  := $(srcdir_)
files_   := main rgbe
subtgt_  := hdr2mip
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

srcdir_  := tools/fb2exr
tgtdir_  := $(srcdir_)
files_   := main
subtgt_  := fb2exr
cflags_  := $(shell pkg-config --cflags OpenEXR)
ldflags_ := $(shell pkg-config --libs OpenEXR) -lscene
$(eval $(call submodule_cpp))

#save and reset
main_programs := $(all_targets)
main_objects := $(all_objects)
all_targets :=
all_objects :=

#------------------------------------------------------------------------------
#TEST PROGRAMS
tests/Test.o: tests/Test.c
	$(CC) $(CFLAGS) -c -o $@ $<
all_objects += tests/Test.o
addobj_ := tests/Test.o

srcdir_  := tests
tgtdir_  := $(srcdir_)
files_   := Array_test
subtgt_  := $(files_)
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

srcdir_  := tests
tgtdir_  := $(srcdir_)
files_   := Numeric_test
subtgt_  := $(files_)
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

srcdir_  := tests
tgtdir_  := $(srcdir_)
files_   := Vector_test
subtgt_  := $(files_)
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

#save and reset
check_programs := $(all_targets)
check_objects := $(all_objects)
all_targets :=
all_objects :=

#------------------------------------------------------------------------------
all_: $(all_depends) $(main_programs)

dependencies := $(subst .o,.d, $(main_objects) )

check: all_ $(check_programs)
	@for t in $(check_programs); \
	do echo running :$$t; $$t; \
	done;

clean:
	-rm -f $(main_programs)
	-rm -f $(main_objects)
	-rm -f $(check_programs)
	-rm -f $(check_objects)
	-rm -f $(all_depends)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(dependencies)
endif

