#Copyright (c) 2011-2013 Hiroshi Tsubokawa
#See LICENSE and README

CC = gcc
OPT = -O3
CFLAGS = $(OPT) -fPIC -Wall -ansi -pedantic-errors
LDFLAGS = -shared -ldl -lm
RM = rm -f

topdir      := ..
target_dir  := lib
target_name := libscene.so
files       := Accelerator Array Box BVHAccelerator Callback Camera Color Curve CurveIO \
	Filter FrameBuffer FrameBufferIO GridAccelerator ImportanceSampling Interval IO \
	Light Matrix Mesh MeshIO Mipmap Noise Numeric ObjectGroup ObjectInstance OS Plugin \
	PointCloud PointCloudIO PrimitiveSet Procedure Progress Property Random Rectangle \
	Renderer Sampler Scene SceneInterface Shader StringFunction SL TexCoord Texture Tiler \
	Timer Transform Triangle Turbulence Vector Volume VolumeAccelerator VolumeFilling

incdir  := $(topdir)/src
libdir  := $(topdir)/lib
target  := $(topdir)/$(target_dir)/$(target_name)

sources := $(addsuffix .c, $(files))
objects := $(addsuffix .o, $(files))
depends := $(addsuffix .d, $(files))

.PHONY: all clean depend
all: $(target)

$(objects): %.o: %.c
	@echo '  compile $<'
	@$(CC) $(CFLAGS) -c -o $@ $<

$(target): $(objects)
	@echo '  link $(target_name)'
	@$(CC) -o $@ $^ $(LDFLAGS)

$(depends): %.d: %.c
	@echo '  dependency $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -MM $< > $@

clean:
	@echo '  clean $(target_name)'
	@$(RM) $(target) $(objects) $(depends) 

ifneq "$(MAKECMDGOALS)" "clean"
-include $(depends)
endif
