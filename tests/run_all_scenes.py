#!/usr/bin/env python

# run all example scenes
# Copyright (c) 2011-2020 Hiroshi Tsubokawa

import subprocess

scn_list = [
	'scenes/furry_bunny.scn',
	'scenes/happy_buddhas.scn',
	'scenes/teapot.scn',
	'scenes/xyzrgb_dragon.scn']

py_list = [
	'scenes/bump_mapping.py',
	'scenes/camera_motion_blur.py',
	'scenes/dome_light1.py',
	'scenes/dome_light2.py',
	'scenes/glassy_happy.py',
	'scenes/grid_light.py',
	'scenes/point_cloud.py',
	'scenes/pyro_ball.py',
	'scenes/sphere_light.py',
	'scenes/spline_wisps.py',
	'scenes/subsurface_scattering.py',
	'scenes/surface_wisps.py',
	'scenes/transform_motion_blur.py',
	'scenes/volume_and_bunny.py']

try:
	result = []

	for filepath in scn_list:
		lines = ""
		for line in open(filepath):
			line = line.replace('640 480', '160 120')
			lines += line

		print 'bin/scene', filepath
		proc = subprocess.Popen(['bin/scene'], stdin=subprocess.PIPE,)
		proc.communicate(lines)

		if proc.returncode == -11:
			result.append((filepath, "Segmentation fault"))
		else:
			result.append((filepath, "OK"))

	for filepath in py_list:
		lines = ""
		for line in open(filepath):
			line = line.replace('.Run(', '.Print(')
			line = line.replace('640, 480', '160, 120')
			lines += line

		print 'python', filepath
		proc = subprocess.Popen(['python'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		(stdoutdata, stderrdata) = proc.communicate(lines)

		proc = subprocess.Popen(['bin/scene'], stdin=subprocess.PIPE,)
		proc.communicate(stdoutdata)

		if proc.returncode == -11:
			result.append((filepath, "Segmentation fault"))
		else:
			result.append((filepath, "OK"))

	total = len(result)
	i = 1
	for r in result:
		print '%d/%d: %s: %s' % (i, total, r[0], r[1])
		i += 1

except KeyboardInterrupt:
	print ''
	print ''
	print '===================='
	print 'Rendering terminated'
	print '===================='
	print ''

except OSError, (errno, strerror):
	print 'error: ' + ': ' + strerror

