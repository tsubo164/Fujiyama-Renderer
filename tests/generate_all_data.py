#!/usr/bin/env python

# generate all mesh, crv and mip data for example scenes
# Copyright (c) 2011-2013 Hiroshi Tsubokawa

import subprocess

ply_list = [
	'armadillo',
	'bunny',
	'dome',
	'dragon',
	'floor',
	'grid',
	'happy',
	'horse',
	'sphere',
	'sphere2',
	'teapot',
	'xyzrgb_dragon']

obj_list = ['head']

mesh_list = ['bunny']

hdr_list = [
	'doge2',
	'ennis',
	'forrest_salzburg02',
	'glacier',
	'grace-new',
	'pisa',
	'uffizi-large']

try:
	for name in ply_list:
		src = '../../ply/'  + name + '.ply'
		dst = '../../mesh/' + name + '.mesh'

		print 'bin/ply2mesh', src, dst
		return_code = subprocess.call(['bin/ply2mesh', src, dst])

	for name in obj_list:
		src = '../../obj/'  + name + '.obj'
		dst = '../../mesh/' + name + '.mesh'

		print 'bin/obj2mesh', src, dst
		return_code = subprocess.call(['bin/obj2mesh', src, dst])

	for name in mesh_list:
		src = '../../mesh/' + name + '.mesh'
		dst = '../../crv/'  + name + '.crv'

		print 'bin/curvegen', src, dst
		return_code = subprocess.call(['bin/curvegen', src, dst])

	for name in hdr_list:
		src = '../../hdr/' + name + '.hdr'
		dst = '../../mip/' + name + '.mip'

		print 'bin/hdr2mip', src, dst
		return_code = subprocess.call(['bin/hdr2mip', src, dst])

except OSError, (errno, strerror):
	print 'error: ' + strerror

