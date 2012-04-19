#!/usr/bin/env python

import subprocess

class SceneInterface:
	def __init__(self):
		self.parser = 'scene'
		self.commands = []

	def Print(self):
		for cmd in self.commands:
			print cmd

	def Run(self):
		commands = ''
		for cmd in self.commands:
			commands = commands + cmd + '\n'

		try:
			p = subprocess.Popen('bin/scene', shell=False, stdin=subprocess.PIPE)
			p.communicate(commands)
		except KeyboardInterrupt:
			print ''
			print ''
			print '===================='
			print 'Rendering terminated'
			print '===================='
			print ''
		except OSError, (errno, strerror):
			print 'error: ' + 'bin/scene' + ': ' + strerror

	def OpenPlugin(self, plugin_path):
		cmd = 'OpenPlugin %s' % (plugin_path)
		self.commands.append(cmd)

	def RenderScene(self, renderer):
		cmd = 'RenderScene %s' % (renderer)
		self.commands.append(cmd)

	def SaveFrameBuffer(self, framebuffer, filename):
		cmd = 'SaveFrameBuffer %s %s' % (framebuffer, filename)
		self.commands.append(cmd)

	# Scene interfaces
	def NewObjectInstance(self, name, accelerator):
		cmd = 'NewObjectInstance %s %s' % (name, accelerator)
		self.commands.append(cmd)

	def NewFrameBuffer(self, name, arg):
		cmd = 'NewFrameBuffer %s %s' % (name, arg)
		self.commands.append(cmd)

	def NewRenderer(self, name):
		cmd = 'NewRenderer %s' % (name)
		self.commands.append(cmd)

	def NewTexture(self, name, filename):
		cmd = 'NewTexture %s %s' % (name, filename)
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
		cmd = 'NewMesh %s %s' % (name, filename)
		self.commands.append(cmd)

	def AssignShader(self, object_instance, shader):
		cmd = 'AssignShader %s %s' % (object_instance, shader)
		self.commands.append(cmd)

	def AssignTexture(self, shader, prop_name, texture):
		cmd = 'AssignTexture %s %s %s' % (shader, prop_name, texture)
		self.commands.append(cmd)

	def AssignCamera(self, renderer, camera):
		cmd = 'AssignCamera %s %s' % (renderer, camera)
		self.commands.append(cmd)

	def AssignFrameBuffer(self, renderer, framebuffer):
		cmd = 'AssignFrameBuffer %s %s' % (renderer, framebuffer)
		self.commands.append(cmd)

	# Property interfaces
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

if __name__ == '__main__':
	si = SceneInterface()

