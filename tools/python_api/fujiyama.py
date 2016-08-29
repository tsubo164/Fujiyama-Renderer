#!/usr/bin/env python

#Copyright (c) 2011-2015 Hiroshi Tsubokawa
#See LICENSE and README

import subprocess
import platform
import tempfile
import shutil
import uuid
import sys
import os

class SceneInterface:

	def __init__(self):
		self.parser = 'scene'
		self.commands = []
		self.pre_conversions = []
		self.post_conversions = []
		self.tempdir = ''

	def __del__(self):
		if self.tempdir != '':
			try:
				shutil.rmtree(self.tempdir)
				print '# Deleting temp directory'
				print self.tempdir
			except OSError:
				print self.tempdir
				print 'No such file or directory'
				print 'Seems that temp directory was removed while rendering'
		print ''

	def Print(self):
		"""
		Prints scene interface commands. No command is excuted.
		"""
		for conv in self.pre_conversions:
			conv_cmd = ''
			for arg in conv:
				conv_cmd += arg + ' '
			print conv_cmd

		for cmd in self.commands:
			print cmd

	def Run(self):
		"""
		Runs scene parser with input stream
		"""

		if self.pre_conversions:
			print '# Running pre conversions'

		for conv in self.pre_conversions:
			try:
				conv_cmd = ''
				for arg in conv:
					conv_cmd += arg + ' '
				print conv_cmd
				code = subprocess.check_call(conv)
			except KeyboardInterrupt:
				print ''
				print ''
				print '===================='
				print 'Conversion terminated'
				print '===================='
				print ''
				sys.exit()
			except OSError, (errno, strerror):
				print 'error: ' + 'scene' + ': ' + strerror
				sys.exit()
			except subprocess.CalledProcessError as e:
				sys.exit()
		print ''

		commands = ''
		for cmd in self.commands:
			commands = commands + cmd + '\n'

		try:
			p = subprocess.Popen('scene', shell=False, stdin=subprocess.PIPE)
			p.communicate(commands)
		except KeyboardInterrupt:
			print ''
			print ''
			print '===================='
			print 'Rendering terminated'
			print '===================='
			print ''
			sys.exit()
		except OSError, (errno, strerror):
			print 'error: ' + 'scene' + ': ' + strerror
			sys.exit()

		if self.post_conversions:
			print '# Running post conversions'

		for conv in self.post_conversions:
			try:
				conv_cmd = ''
				for arg in conv:
					conv_cmd += arg + ' '
				print conv_cmd
				code = subprocess.check_call(conv)
			except KeyboardInterrupt:
				print ''
				print ''
				print '===================='
				print 'Conversion terminated'
				print '===================='
				print ''
				sys.exit()
			except OSError, (errno, strerror):
				print 'error: ' + 'scene' + ': ' + strerror
				sys.exit()
			except subprocess.CalledProcessError as e:
				sys.exit()
		print ''

	def Comment(self, comment):
		"""
		Put a comment on scene script. Max munber of characters is 128.
		"""
		cmd = '# %.128s' % (comment)
		self.commands.append(cmd)

	def OpenPlugin(self, plugin_path):
		"""
		Opens a plugin. DSO extension (.so, .dll, ...) will be added when missing
		"""
		root, plugin_ext = os.path.splitext(plugin_path)
		if os.name == 'posix':
			dso_ext = '.so'
		elif os.name == 'nt':
			dso_ext = '.dll'

		path = plugin_path
		if plugin_ext != dso_ext:
			path += dso_ext

		cmd = 'OpenPlugin %s' % (path)
		self.commands.append(cmd)

	def RenderScene(self, renderer):
		cmd = 'RenderScene %s' % (renderer)
		self.commands.append(cmd)

	def SaveFrameBuffer(self, framebuffer, filename):
		filepath, ext = os.path.splitext(filename)

		if ext == '.fb':
			temp_filename = filename
		elif ext == '.exr':
			temp_filename = self.__setup_post_conversion('fb2exr', filename, '.fb', '.exr')
		else:
			temp_filename = filename
			print 'non supported texture file format'

		cmd = 'SaveFrameBuffer %s %s' % (framebuffer, temp_filename)
		self.commands.append(cmd)

	def RunProcedure(self, procedure):
		cmd = 'RunProcedure %s' % (procedure)
		self.commands.append(cmd)

	def AddObjectToGroup(self, group, object):
		cmd = 'AddObjectToGroup %s %s' % (group, object)
		self.commands.append(cmd)

	def NewObjectInstance(self, name, accelerator):
		cmd = 'NewObjectInstance %s %s' % (name, accelerator)
		self.commands.append(cmd)

	def NewFrameBuffer(self, name, arg):
		cmd = 'NewFrameBuffer %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewObjectGroup(self, name):
		cmd = 'NewObjectGroup %s' % (name)
		self.commands.append(cmd)

	def NewPointCloud(self, name, filename):
		cmd = 'NewPointCloud %s %s' % (name, filename)
		self.commands.append(cmd)

	def NewTurbulence(self, name):
		cmd = 'NewTurbulence %s' % (name)
		self.commands.append(cmd)

	def NewProcedure(self, name, arg):
		cmd = 'NewProcedure %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewRenderer(self, name):
		cmd = 'NewRenderer %s' % (name)
		self.commands.append(cmd)

	def NewTexture(self, name, filename):
		filepath, ext = os.path.splitext(filename)

		if ext == '.mip':
			temp_filename = filename
		elif ext == '.hdr':
			temp_filename = self.__setup_pre_conversion('hdr2mip', filename, '.hdr', '.mip')
		elif ext == '.jpg':
			temp_filename = self.__setup_pre_conversion('jpg2mip', filename, '.jpg', '.mip')
		else:
			temp_filename = filename
			print 'non supported texture file format'

		cmd = 'NewTexture %s %s' % (name, temp_filename)
		self.commands.append(cmd)

	def NewCamera(self, name, arg):
		cmd = 'NewCamera %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewShader(self, name, arg):
		cmd = 'NewShader %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewVolume(self, name):
		cmd = 'NewVolume %s' % (name)
		self.commands.append(cmd)

	def NewCurve(self, name, filename):
		cmd = 'NewCurve %s %s' % (name, filename)
		self.commands.append(cmd)

	def NewLight(self, name, arg):
		cmd = 'NewLight %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewMesh(self, name, filename):
		filepath, ext = os.path.splitext(filename)

		if ext == '.mesh':
			temp_filename = filename
		elif ext == '.ply':
			temp_filename = self.__setup_pre_conversion('ply2mesh', filename, '.ply', '.mesh')
		elif ext == '.obj':
			temp_filename = self.__setup_pre_conversion('obj2mesh', filename, '.obj', '.mesh')
		else:
			temp_filename = filename
			print 'non supported texture file format'

		cmd = 'NewMesh %s %s' % (name, temp_filename)
		self.commands.append(cmd)

	def AssignShader(self, object_instance, shading_group, shader):
		cmd = 'AssignShader %s %s %s' % (object_instance, shading_group, shader)
		self.commands.append(cmd)

	def AssignTexture(self, shader, prop_name, texture):
		cmd = 'AssignTexture %s %s %s' % (shader, prop_name, texture)
		self.commands.append(cmd)

	def AssignCamera(self, renderer, camera):
		cmd = 'AssignCamera %s %s' % (renderer, camera)
		self.commands.append(cmd)

	def AssignObjectGroup(self, entry_name, prop_name, object_group):
		cmd = 'AssignObjectGroup %s %s %s' % (entry_name, prop_name, object_group)
		self.commands.append(cmd)

	def AssignFrameBuffer(self, renderer, framebuffer):
		cmd = 'AssignFrameBuffer %s %s' % (renderer, framebuffer)
		self.commands.append(cmd)

	def AssignTurbulence(self, entry_name, prop_name, turbulence):
		cmd = 'AssignTurbulence %s %s %s' % (entry_name, prop_name, turbulence)
		self.commands.append(cmd)

	def AssignVolume(self, entry_name, prop_name, volume):
		cmd = 'AssignVolume %s %s %s' % (entry_name, prop_name, volume)
		self.commands.append(cmd)

	def AssignMesh(self, entry_name, prop_name, mesh):
		cmd = 'AssignMesh %s %s %s' % (entry_name, prop_name, mesh)
		self.commands.append(cmd)

	def SetProperty1(self, entry_name, prop_name, v0):
		cmd = 'SetProperty1 %s %s %s' % (entry_name, prop_name, v0)
		self.commands.append(cmd)

	def SetProperty2(self, entry_name, prop_name, v0, v1):
		cmd = 'SetProperty2 %s %s %s %s' % (entry_name, prop_name, v0, v1)
		self.commands.append(cmd)

	def SetProperty3(self, entry_name, prop_name, v0, v1, v2):
		cmd = 'SetProperty3 %s %s %s %s %s' % (entry_name, prop_name, v0, v1, v2)
		self.commands.append(cmd)

	def SetProperty4(self, entry_name, prop_name, v0, v1, v2, v3):
		cmd = 'SetProperty4 %s %s %s %s %s %s' % (entry_name, prop_name, v0, v1, v2, v3)
		self.commands.append(cmd)

	def SetStringProperty(self, entry_name, prop_name, string):
		cmd = 'SetStringProperty %s %s %s' % (entry_name, prop_name, string)
		self.commands.append(cmd)

	def SetSampleProperty3(self, entry_name, prop_name, v0, v1, v2, time):
		cmd = 'SetSampleProperty3 %s %s %s %s %s %s' % (entry_name, prop_name, v0, v1, v2, time)
		self.commands.append(cmd)

	def ShowPropertyList(self, type_name):
		"""
		Show property list of type (ObjectInstance, Volume, ...) or
		plugin (PlasticShader, ...). When showing list of plugin properties,
		the plugin must be opened before this command.
		"""
		cmd = 'ShowPropertyList %s' % (type_name)
		self.commands.append(cmd)

	def __setup_pre_conversion(self, converter, orig_filename, from_ext, to_ext):
		if self.tempdir == '':
			self.tempdir = tempfile.mkdtemp()
			print '# Creating temp directory'
			print self.tempdir
			print

		temp_filename = os.path.basename(orig_filename)
		temp_filename = temp_filename.replace(from_ext, to_ext)
		temp_filename = str(uuid.uuid4()) + '_' + temp_filename
		temp_filename = os.path.join(self.tempdir, temp_filename)
		self.pre_conversions.append([converter, orig_filename, temp_filename])
		return temp_filename

	def __setup_post_conversion(self, converter, orig_filename, from_ext, to_ext):
		if self.tempdir == '':
			self.tempdir = tempfile.mkdtemp()
			print '# Creating temp directory'
			print self.tempdir
			print

		temp_filename = os.path.basename(orig_filename)
		temp_filename = temp_filename.replace(to_ext, from_ext)
		temp_filename = str(uuid.uuid4()) + '_' + temp_filename
		temp_filename = os.path.join(self.tempdir, temp_filename)
		self.post_conversions.append([converter, temp_filename, orig_filename])
		return temp_filename

def setup_environment():
	fj_lib_path = 'FJ_LIBRARY_PATH'
	platform_name = platform.system()
	lib_path = ''

	if platform_name == 'Darwin':
		lib_path = 'DYLD_LIBRARY_PATH'
	elif platform_name == 'Linux':
		lib_path = 'DYLD_LIBRARY_PATH'
	elif platform_name == 'Windows':
		lib_path = 'PATH'
	else:
		print
		print '# ERROR! unknown platform'
		print
		sys.exit()

	if os.environ[fj_lib_path] == '':
		print
		print '# ERROR!', fj_lib_path, 'environment variable is not set up properly'
		print
		sys.exit()
	else:
		print
		print '# Setting up environment variable'
		os.environ[lib_path] = os.environ[fj_lib_path]
		print '#', fj_lib_path, ' =', os.environ[lib_path]
		print

setup_environment()

if __name__ == '__main__':
	si = SceneInterface()
