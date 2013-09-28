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
files       := \
		fj_accelerator fj_array fj_box fj_bvh_accelerator fj_callback fj_camera \
		fj_color fj_curve fj_curve_io fj_file_io fj_filter fj_framebuffer \
		fj_framebuffer_io fj_grid_accelerator fj_importance_sampling fj_interval \
		fj_io fj_light fj_matrix fj_mesh fj_mesh_io fj_mipmap fj_noise fj_numeric \
		fj_object_group fj_object_instance fj_os fj_plugin fj_point_cloud fj_point_cloud_io \
		fj_primitive_set fj_procedure fj_progress fj_property fj_random fj_rectangle \
		fj_renderer fj_sampler fj_scene fj_scene_interface fj_shader fj_string_function \
		fj_shading fj_tex_coord fj_texture fj_tiler fj_timer fj_transform fj_triangle \
		fj_turbulence fj_vector fj_volume fj_volume_accelerator fj_volume_filling

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
