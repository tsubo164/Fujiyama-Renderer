#!/usr/bin/env python
#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE and README

#import subprocess
import sys, os
from glob import glob

top_dir = '../..'
src_dir = top_dir + '/src'
nmake_file = 'Makefile'

write_dependencies = False

header = """\
# nmake file generated by {filename}
# Copyright (c) 2011-2014 Hiroshi Tsubokawa
# See LICENSE and README
""".format(filename = os.path.basename(__file__))

dirs = """\
INCLUDE_PATH = C:\\include
LIBRARY_PATH = C:\\lib

out_dir = bin
"""

opts = """\
opt = /O2 /Oi /GL
warn = /wd4351 #/W3
macro = /D "NODEBUG"
"""

aliases = """\
CC = cl.exe
LD = link.exe
CXXFLAGS = /nologo $(opt) $(warn) $(macro) /fp:precise /EHsc /MD /openmp /I{src_dir} /I$(INCLUDE_PATH) /c
LDFLAGS = /nologo /LTCG /LIBPATH:$(out_dir) /LIBPATH:$(LIBRARY_PATH)
RM = del
""".format(src_dir = src_dir.replace('/','\\'))

cxx_action = """\
	@$(CC) $(CXXFLAGS) {additional_cflags} /Fo$@ {source_file}"""

link_action = """\
	@echo {name}
	@$(LD) $(LDFLAGS) /out:$@ {additional_ldflags} {additional_libs} $({object_list_macro})"""

phony = """\
.PHONY: all clean check
"""

target_list = [
# libscene
{
	'name':               'libscene.dll',
	'source_list':        glob(top_dir + '/src/*.cc'),
	'additional_cflags':  '/D "FJ_DLL_EXPORT"',
	'additional_ldflags': '/DLL',
	'additional_libs':    '',
},
# sample
{
	'name':               'cube.exe',
	'source_list':        glob(top_dir + '/scenes/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
# shaders
{
	'name':               'ConstantShader.dll',
	'source_list':        glob(top_dir + '/shaders/constant_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'GlassShader.dll',
	'source_list':        glob(top_dir + '/shaders/glass_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'HairShader.dll',
	'source_list':        glob(top_dir + '/shaders/hair_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'PlasticShader.dll',
	'source_list':        glob(top_dir + '/shaders/plastic_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'SSSShader.dll',
	'source_list':        glob(top_dir + '/shaders/sss_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'VolumeShader.dll',
	'source_list':        glob(top_dir + '/shaders/volume_shader/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
# procedures
{
	'name':               'PointCloudsProcedure.dll',
	'source_list':        glob(top_dir + '/procedures/pointclouds_procedure/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'SplineWispsProcedure.dll',
	'source_list':        glob(top_dir + '/procedures/splinewisps_procedure/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'SurfaceWispsProcedure.dll',
	'source_list':        glob(top_dir + '/procedures/surfacewisps_procedure/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '/DLL',
	'additional_libs':    'libscene.lib',
},
# tools
{
	'name':               'curvegen.exe',
	'source_list':        glob(top_dir + '/tools/curve_generator/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'fb2exr.exe',
	'source_list':        glob(top_dir + '/tools/fb2exr/*.cc'),
	'additional_cflags':  '/D "OPENEXR_DLL"',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib Half.lib Iex.lib IlmImf.lib IlmThread.lib Imath.lib zlibwapi.lib',
},
{
	'name':               'fbview.exe',
	'source_list':        glob(top_dir + '/tools/framebuffer_viewer/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'hdr2mip.exe',
	'source_list':        glob(top_dir + '/tools/hdr2mip/*.cc') + glob(top_dir + '/tools/hdr2mip/*.c'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'jpg2mip.exe',
	'source_list':        glob(top_dir + '/tools/jpg2mip/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib jpeg.lib',
},
{
	'name':               'obj2mesh.exe',
	'source_list':        glob(top_dir + '/tools/obj2mesh/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'ply2mesh.exe',
	'source_list':        glob(top_dir + '/tools/ply2mesh/*.cc') + glob(top_dir + '/tools/ply2mesh/*.c'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'ptcgen.exe',
	'source_list':        glob(top_dir + '/tools/point_cloud_generator/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'scene.exe',
	'source_list':        glob(top_dir + '/tools/scene_parser/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'velgen.exe',
	'source_list':        glob(top_dir + '/tools/velocity_generator/*.cc'),
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
# units tests
{
	# only the first test target has the rule for unit_test.obj
	# the rest have unit_test.obj in additional_ldflags
	'name':               'box_test.exe',
	'source_list':        [top_dir + '/tests/unit_test.cc', top_dir + '/tests/box_test.cc'],
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib',
},
{
	'name':               'io_test.exe',
	'source_list':        [top_dir + '/tests/io_test.cc'],
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib ' + top_dir + '/tests/unit_test.obj',
},
{
	'name':               'numeric_test.exe',
	'source_list':        [top_dir + '/tests/numeric_test.cc'],
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib ' + top_dir + '/tests/unit_test.obj',
},
{
	'name':               'vector_test.exe',
	'source_list':        [top_dir + '/tests/vector_test.cc'],
	'additional_cflags':  '',
	'additional_ldflags': '',
	'additional_libs':    'libscene.lib ' + top_dir + '/tests/unit_test.obj',
},
]

def fprint(f, header):
	f.write(header)

def make_target_header(target_name):
	text = '#' + '=' * 79 + '\n'
	return text

def init_target_list(target_list):
	for target in target_list:
		target['name_macro'] = target['name'].replace('.','_')
		target['object_list_macro'] = target['name_macro'] + '_obj'

		new_src_list = []
		for src in target['source_list']:
			new_src_list.append(src.replace('/', '\\'))
		target['source_list'] = new_src_list

		target['object_list'] = []
		for src in target['source_list']:
			if src.endswith('.cc'):
				target['object_list'].append(src.replace('.cc', '.obj'))
			else:
				target['object_list'].append(src.replace('.c', '.obj'))

		target['lib_macro'] = ''
		target['exp_macro'] = ''
		if target['name'].endswith('.dll'):
			target['lib_macro'] = target['name_macro'] + '_lib'
			target['exp_macro'] = target['name_macro'] + '_exp'

def make_macro_list():
	text = ''
	text += header
	text += '\n'
	text += dirs
	text += '\n'
	text += opts
	text += '\n'
	text += aliases
	return text

def make_target_macro_list(target_list):
	text = ''
	for target in target_list:
		text += target['name_macro'] + ' = ' + '$(out_dir)\\' + target['name'] + '\n'
	return text

def make_target_all(target_list):
	text = ''
	text += make_target_header('all')
	text += 'all:'
	for target in target_list:
		text += ' \\\n  $(' + target['name_macro'] + ')'
	text += '\n'
	return text

def make_target(target):
	text = ''
	text += make_target_header(target['name'])
	text += target['object_list_macro'] + ' ='
	for obj in target['object_list']:
		text += ' \\\n  ' + obj
	text += '\n\n'

	for (obj, src) in zip(target['object_list'], target['source_list']):
		text += obj + ' : ' + src + '\n'
		text += cxx_action.format(
			additional_cflags = target['additional_cflags'],
			source_file = src) + '\n\n'

	text += '$(' + target['name_macro'] + ') : $(' + target['object_list_macro'] + ')\n'
	text += link_action.format(
		name = target['name'],
		additional_ldflags = target['additional_ldflags'],
		additional_libs = target['additional_libs'],
		object_list_macro = target['object_list_macro'])
	text += '\n'

	if target['lib_macro']:
		text += '\n'
		text += target['lib_macro'] + ' = $(out_dir)\\' + target['lib_macro'] + '\n'
		text += target['exp_macro'] + ' = $(out_dir)\\' + target['exp_macro'] + '\n'

	return text

def make_target_obj(src, obj, additional_cflags):
	text = ''
	text += obj + ' : ' + src + '\n'
	text += cxx_action.format(
		additional_cflags = additional_cflags,
		source_file = src)
	text += '\n'
	return text

def make_target_check(target_list):
	text = ''
	text += make_target_header('check')
	text += 'check:\n'
	for target in target_list:
		if target['name_macro'].endswith('_test_exe'):
			text += '\t@$(' + target['name_macro'] + ')\n'
	return text

def make_target_clean(target_list):
	text = ''
	text += make_target_header('clean')
	text += 'clean:\n'
	for target in target_list:
		text += '\t$(RM) $(' + target['name_macro'] + ')\n'
		text += '\t$(RM) $(' + target['object_list_macro'] + ')\n'
		if target['lib_macro']:
			text += '\t$(RM) $(' + target['lib_macro'] + ')\n'
		if target['exp_macro']:
			text += '\t$(RM) $(' + target['exp_macro'] + ')\n'
	return text

if __name__ == '__main__':
	f = open(nmake_file, 'w')
	#f = sys.stdout

	init_target_list(target_list)

	fprint(f, make_macro_list())
	fprint(f, '\n')

	fprint(f, make_target_macro_list(target_list))
	fprint(f, '\n')

	fprint(f, make_target_all(target_list))
	fprint(f, '\n')

	fprint(f, phony)
	fprint(f, '\n')

	for target in target_list:
		fprint(f, make_target(target))
		fprint(f, '\n')

	fprint(f, make_target_check(target_list))
	fprint(f, '\n')

	fprint(f, make_target_clean(target_list))
	fprint(f, '\n')

