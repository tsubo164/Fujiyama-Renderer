#Copyright (c) 2011-2020 Hiroshi Tsubokawa
#See LICENSE and README

CC = g++
OPT = -O3
CFLAGS = $(OPT) -fPIC -Wall -std=c++11 -pedantic-errors
LDFLAGS = -shared -ldl -lm
RM = rm -f

#XXX compatibility for Linux pthread
ifeq ($(shell uname),Linux)
LDFLAGS = -shared -ldl -lm -pthread
endif

topdir      := ..
target_dir  := lib
target_name := libscene.so
files       := \
		fj_accelerator fj_adaptive_grid_sampler fj_box fj_bvh_accelerator fj_callback \
		fj_camera fj_curve fj_dome_light fj_filter fj_fixed_grid_sampler fj_framebuffer \
		fj_framebuffer_io fj_geometry fj_geometry_io fj_grid_accelerator \
		fj_importance_sampling fj_interval fj_light fj_matrix fj_mesh \
		fj_mipmap fj_multi_thread fj_noise fj_object_group fj_object_instance \
		fj_object_set fj_os fj_plugin fj_primitive_set fj_point_cloud fj_point_light \
		fj_procedure fj_progress fj_property fj_protocol fj_random fj_rectangle \
		fj_rectangle_light fj_renderer fj_sampler fj_scene fj_scene_interface fj_scene_node \
		fj_shader fj_shading fj_socket fj_sphere_light fj_texture fj_tiler fj_timer \
		fj_transform fj_triangle fj_turbulence fj_volume fj_volume_accelerator \
		fj_volume_filling

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
	@$(CC) $(CFLAGS) -c -o $@ $<

$(target): $(objects)
	@echo '  link $(target_name)'
	@$(CC) -o $@ $^ $(LDFLAGS)

$(depends): %.d: %.cc
	@echo '  dependency $<'
	@$(CC) $(CFLAGS) -I$(incdir) -c -MM $< > $@

clean:
	@echo '  clean $(target_name)'
	@$(RM) $(target) $(objects) $(depends) 

ifneq "$(MAKECMDGOALS)" "clean"
-include $(depends)
endif
