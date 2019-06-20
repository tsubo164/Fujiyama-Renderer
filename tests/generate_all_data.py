#!/usr/bin/env python

# generate all mesh, crv and mip data for example scenes
# Copyright (c) 2011-2019 Hiroshi Tsubokawa

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
	'teapot',
	'xyzrgb_dragon']

obj_list = ['head']

mesh_list = ['bunny']

hdr_list = [
	'austria',
	'doge2',
	'ennis',
	'forrest_salzburg02',
	'glacier',
	'grace-new',
	'pisa',
	'uffizi-large']

jpg_list = [
	'rock',
	'rust',
	'concrete',
	'pattern']

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

	for name in mesh_list:
		src = '../../mesh/' + name + '.mesh'
		dst = '../../ptc/'  + name + '.ptc'

		print 'bin/ptcgen', src, dst
		return_code = subprocess.call(['bin/ptcgen', src, dst])

	for name in hdr_list:
		src = '../../hdr/' + name + '.hdr'
		dst = '../../mip/' + name + '.mip'

		print 'bin/hdr2mip', src, dst
		return_code = subprocess.call(['bin/hdr2mip', src, dst])

	for name in jpg_list:
		src = '../../jpg/' + name + '.jpg'
		dst = '../../mip/' + name + '.mip'

		print 'bin/jpg2mip', src, dst
		return_code = subprocess.call(['bin/jpg2mip', src, dst])

except OSError, (errno, strerror):
	print 'error: ' + strerror

