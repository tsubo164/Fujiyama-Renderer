#Copyright (c) 2011-2013 Hiroshi Tsubokawa
#See LICENSE and README

CC = gcc
CXX = g++
OPT = -O3
CFLAGS = -Isrc -Wall -ansi -pedantic-errors $(OPT)
LDFLAGS = -Llib
CPPFLAGS = -Isrc -Wall $(OPT)

RM = rm -f
INSTALL = install

.PHONY: all all_ clean install install_library install_shaders install_procedures \
		install_bin install_tools sample scenes/cube.fb
all: all_

prefix = /usr/local

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
	@echo '  compile $$<'
	@$$(CC) $$(CFLAGS) $(cflags_) -c -o $$@ $$<
$$(target): $$(objects) $(addobj_)
	@echo '  link $$^'
	@$$(CC) -o $$@ $$^ $$(LDFLAGS) $(ldflags_)
$$(depends): %.d: %.c
	@echo '  dependency $$<'
	@$$(CC) $$(CFLAGS) $(cflags_) -c -MM $$< | \
	sed 's,\($$(notdir $$*)\.o\) *:,$$(dir $$@)\1 $@: ,' > $$@.tmp
	@mv $$@.tmp $$@
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
	@echo '  compile $$<'
	@$$(CXX) $$(CPPFLAGS) $(cflags_) -c -o $$@ $$<
$$(target): $$(objects) $(addobj_)
	@echo '  link $$^'
	@$$(CXX) -o $$@ $$^ $$(LDFLAGS) $(ldflags_)
$$(depends): %.d: %.cpp
	@echo '  dependency $$<'
	@$$(CXX) $$(CPPFLAGS) $(cflags_) -c -MM $$< | \
	sed 's,\($$(notdir $$*)\.o\) *:,$$(dir $$@)\1 $@: ,' > $$@.tmp
	@mv $$@.tmp $$@
endef
#******************************************************************************

all_targets :=
all_objects :=
all_depends :=
addobj_ :=
install_lib :=
install_bin :=
install_shaders :=
install_procedures :=

#core library
srcdir_  := src
tgtdir_  := lib
files_   := Accelerator Array Box Camera Curve CurveIO Filter FrameBuffer FrameBufferIO \
	ImportanceSampling Intersection Interval IO Light Matrix Mesh MeshIO Mipmap Noise \
	Numeric ObjectGroup ObjectInstance OS Plugin PrimitiveSet Procedure Progress Property \
	Random Renderer Sampler Scene SceneInterface Shader String SL Texture Tiler Timer \
	Transform Triangle Turbulence Volume VolumeAccelerator VolumeFilling

subtgt_  := libscene.so
cflags_  := -fPIC
ldflags_ := -shared -ldl -lm
$(eval $(call submodule))

#install
install_lib := $(subtgt_)

#shaders
srcdir_  := shaders/PlasticShader
tgtdir_  := lib
files_   := PlasticShader
subtgt_  := PlasticShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

srcdir_  := shaders/GlassShader
tgtdir_  := lib
files_   := GlassShader
subtgt_  := GlassShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

srcdir_  := shaders/ConstantShader
tgtdir_  := lib
files_   := ConstantShader
subtgt_  := ConstantShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

srcdir_  := shaders/HairShader
tgtdir_  := lib
files_   := HairShader
subtgt_  := HairShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

srcdir_  := shaders/VolumeShader
tgtdir_  := lib
files_   := VolumeShader
subtgt_  := VolumeShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

srcdir_  := shaders/SSSShader
tgtdir_  := lib
files_   := SSSShader
subtgt_  := SSSShader.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_shaders += $(subtgt_)

#procedures
srcdir_  := procedures/PointCloudsProcedure
tgtdir_  := lib
files_   := PointCloudsProcedure
subtgt_  := PointCloudsProcedure.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_procedures += $(subtgt_)

srcdir_  := procedures/SplineWispsProcedure
tgtdir_  := lib
files_   := SplineWispsProcedure
subtgt_  := SplineWispsProcedure.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_procedures += $(subtgt_)

srcdir_  := procedures/SurfaceWispsProcedure
tgtdir_  := lib
files_   := SurfaceWispsProcedure
subtgt_  := SurfaceWispsProcedure.so
cflags_  := -fPIC
ldflags_ := -shared -lscene
$(eval $(call submodule))

install_procedures += $(subtgt_)

#tools
srcdir_  := tools/SceneParser
tgtdir_  := bin
files_   := main Parser Table Command
subtgt_  := scene
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

install_bin += $(subtgt_)

srcdir_  := tools/FrameBufferViewer
tgtdir_  := bin
files_   := main FrameBufferViewer
subtgt_  := fbview
cflags_  :=
ldflags_ := -lscene -lGL -lGLU -lglut
$(eval $(call submodule))

install_bin += $(subtgt_)

srcdir_  := tools/ply2mesh
tgtdir_  := bin
files_   := main plyfile
subtgt_  := ply2mesh
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

install_bin += $(subtgt_)

srcdir_  := tools/obj2mesh
tgtdir_  := bin
files_   := main ObjParser
subtgt_  := obj2mesh
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

install_bin += $(subtgt_)

srcdir_  := tools/hdr2mip
tgtdir_  := bin
files_   := main rgbe
subtgt_  := hdr2mip
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

install_bin += $(subtgt_)

srcdir_  := tools/fb2exr
tgtdir_  := bin
files_   := main
subtgt_  := fb2exr
cflags_  := $(shell pkg-config --cflags OpenEXR)
ldflags_ := $(shell pkg-config --libs OpenEXR) -lscene
$(eval $(call submodule_cpp))

install_bin += $(subtgt_)

srcdir_  := tools/CurveGenerator
tgtdir_  := bin
files_   := main
subtgt_  := curvegen
cflags_  :=
ldflags_ := -lscene
$(eval $(call submodule))

install_bin += $(subtgt_)

#save and reset
main_programs := $(all_targets)
main_objects := $(all_objects)
all_targets :=
all_objects :=

pyc_files := tools/PythonAPI/fujiyama.pyc

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

srcdir_  := tests
tgtdir_  := $(srcdir_)
files_   := Box_test
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
#Sample Programs
sample: scenes/cube.fb bin/fbview
	env LD_LIBRARY_PATH=lib bin/fbview $<
scenes/cube.fb: scenes/cube scenes/cube.mesh
	env LD_LIBRARY_PATH=lib scenes/cube
scenes/cube: scenes/cube.c lib/libscene.so lib/PlasticShader.so
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lscene
scenes/cube.mesh: scenes/cube.ply bin/ply2mesh
	env LD_LIBRARY_PATH=lib bin/ply2mesh $< $@
sample_outputs := scenes/cube.fb scenes/cube.mesh scenes/cube

#------------------------------------------------------------------------------
all_: $(all_depends) $(main_programs)

dependencies := $(subst .o,.d, $(main_objects) )

check: all_ $(check_programs)
	@for t in $(check_programs); \
	do echo running :$$t; $$t; \
	done;

install: all_ install_library install_shaders install_tools

install_library:
	@echo '  install' lib/$(install_lib)
	@$(INSTALL) -d -m 755 $(prefix)/lib
	@$(INSTALL) -m 755 lib/$(install_lib) $(prefix)/lib/$(install_lib)

install_shaders:
	@$(INSTALL) -d -m 755 $(prefix)/lib
	@for t in $(install_shaders); \
	do echo '  install' lib/$$t; \
		$(INSTALL) -m 755 lib/$$t $(prefix)/lib/$$t; \
	done;

install_tools:
	@$(INSTALL) -d -m 755 $(prefix)/bin
	@for t in $(install_bin); \
	do echo '  install' bin/$$t; \
		$(INSTALL) -m 755 bin/$$t $(prefix)/bin/$$t; \
	done;

clean:
	-$(RM) $(main_programs)
	-$(RM) $(main_objects)
	-$(RM) $(check_programs)
	-$(RM) $(check_objects)
	-$(RM) $(all_depends)
	-$(RM) $(sample_outputs)
	-$(RM) $(pyc_files)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(dependencies)
endif

