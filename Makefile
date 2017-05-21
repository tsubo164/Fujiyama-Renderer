# Copyright (c) 2011-2017 Hiroshi Tsubokawa
# See LICENSE and README

MAKE    = make --no-print-directory
INSTALL = install
RM      = rm -f

prefix       := /usr/local
build_dirs   :=
clean_dirs   :=
install_bins :=
install_libs :=

#core
build_dirs   += src
install_libs += lib/libscene.so

#shaders
build_dirs   += shaders/constant_shader
install_libs += lib/ConstantShader.so

build_dirs   += shaders/glass_shader
install_libs += lib/GlassShader.so

build_dirs   += shaders/hair_shader
install_libs += lib/HairShader.so

build_dirs   += shaders/pathtracing_shader
install_libs += lib/PathtracingShader.so

build_dirs   += shaders/plastic_shader
install_libs += lib/PlasticShader.so

build_dirs   += shaders/sss_shader
install_libs += lib/SSSShader.so

build_dirs   += shaders/volume_shader
install_libs += lib/VolumeShader.so

#procedures
build_dirs   += procedures/constantvolume_procedure
install_libs += lib/ConstantVolumeProcedure.so

build_dirs   += procedures/pointclouds_procedure
install_libs += lib/PointCloudsProcedure.so

build_dirs   += procedures/splinewisps_procedure
install_libs += lib/SplineWispsProcedure.so

build_dirs   += procedures/surfacewisps_procedure
install_libs += lib/SurfaceWispsProcedure.so

build_dirs   += procedures/curve_generator_procedure
install_libs += lib/CurveGeneratorProcedure.so

build_dirs   += procedures/pointcloud_generator
install_libs += lib/PointcloudGenerator.so

build_dirs   += procedures/velocity_generator_procedure
install_libs += lib/VelocityGeneratorProcedure.so

build_dirs   += procedures/stanfordply_procedure
install_libs += lib/StanfordPlyProcedure.so

build_dirs   += procedures/wavefrontobj_procedure
install_libs += lib/WavefrontObjProcedure.so

#tools
build_dirs   += tools/fb2exr
install_bins += bin/fb2exr

build_dirs   += tools/framebuffer_viewer
install_bins += bin/fbview

build_dirs   += tools/hdr2mip
install_bins += bin/hdr2mip

build_dirs   += tools/jpg2mip
install_bins += bin/jpg2mip

build_dirs   += tools/scene_parser
install_bins += bin/scene

clean_dirs += $(build_dirs)
clean_dirs += tools/python_api

#sample
sample_dir := scenes
clean_dirs += $(sample_dir)

#tests
tests_dir := tests
clean_dirs += $(tests_dir)

.PHONY: all build check sample clean \
		install build install_libraries install_binaries

all: build

build:
	@for t in $(build_dirs); \
	do echo $$t; \
		$(MAKE) -C $$t; \
	done;

sample: build
	@$(MAKE) -C $(sample_dir) $@

check: build
	@$(MAKE) -C $(tests_dir) $@

clean:
	@for t in $(clean_dirs); \
	do echo $$t; \
		$(MAKE) -C $$t clean; \
	done;

install:
	@for t in $(install_bins) $(install_libs); \
	do echo '  install' $(prefix)/$$t; \
		$(INSTALL) -m 755 $$t $(prefix)/$$t; \
	done;

