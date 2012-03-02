#!/usr/bin/env python

#import subprocess

class Fujiyama:
	def __init__(self):
		self.parser = 'scene'
		self.commands = []
		print 'Fujiyama initialized'

	def OpenPlugin(self, plugin_path):
		cmd = 'OpenPlugin ' + plugin_path
		self.commands.append(cmd)

	def Run(self):
		print 'Running fujiyama ...'
		for cmd in self.commands:
			print cmd

	def RenderScene(self, renderer):
		cmd = 'RenderScene ' + renderer
		self.commands.append(cmd)

	def SaveFrameBuffer(self, framebuffer, filename):
		cmd = 'SaveFrameBuffer ' + framebuffer + ' ' + filename
		self.commands.append(cmd)

	# Scene interfaces
	def NewObjectInstance(self, name, accelerator):
		cmd = 'NewObjectInstance ' + name + ' ' + accelerator
		self.commands.append(cmd)

	def NewFrameBuffer(self, name, arg):
		cmd = 'NewFrameBuffer ' + name + ' ' + arg
		self.commands.append(cmd)

	def NewRenderer(self, name):
		cmd = 'NewRenderer ' + name
		self.commands.append(cmd)

	def NewTexture(self, name, filename):
		cmd = 'NewTexture ' + name + ' ' + filename
		self.commands.append(cmd)

	def NewCamera(self, name, arg):
		cmd = 'NewCamera ' + name + ' ' + arg
		self.commands.append(cmd)

	def NewShader(self, name, arg):
		cmd = 'NewShader ' + name + ' ' + arg
		self.commands.append(cmd)

	def NewCurve(self, name, filename):
		cmd = 'NewCurve ' + name + ' ' + filename
		self.commands.append(cmd)

	def NewLight(self, name, arg):
		cmd = 'NewLight ' + name + ' ' + arg
		self.commands.append(cmd)

	def NewMesh(self, name, filename):
		cmd = 'NewMesh ' + name + ' ' + filename
		self.commands.append(cmd)

	def AssignShader(self, object_instance, shader):
		cmd = 'AssignShader ' + object_instance + ' ' + shader
		self.commands.append(cmd)

	def AssignTexture(self, shader, prop_name, texture):
		cmd = 'AssignTexture ' + shader + ' ' + prop_name + ' ' + texture
		self.commands.append(cmd)

	def AssignCamera(self, renderer, camera):
		cmd = 'AssignCamera ' + renderer + ' ' + camera
		self.commands.append(cmd)

	def AssignFrameBuffer(self, renderer, framebuffer):
		cmd = 'AssignFrameBuffer ' + renderer + ' ' + framebuffer
		self.commands.append(cmd)

	# Property interfaces
	def SetProperty1(self, entry_name, prop_name, v0):
		cmd = 'SetProperty1 ' + name + ' ' + prop_name + ' ' + v0
		self.commands.append(cmd)

	def SetProperty2(self, entry_name, prop_name, v0, v1):
		cmd = 'SetProperty2 ' + name + ' ' + prop_name + ' ' + v0 + ' ' + v1
		self.commands.append(cmd)

	def SetProperty3(self, entry_name, prop_name, v0, v1, v2):
		cmd = 'SetProperty3 ' + name + ' ' + prop_name + ' ' + v0 + ' ' + v1 + ' ' + v2
		self.commands.append(cmd)

	def SetProperty4(self, entry_name, prop_name, v0, v1, v2, v3):
		cmd = 'SetProperty4 ' + name + ' ' + prop_name + ' ' + v0 + ' ' + v1 + ' ' + v2 + ' ' + v3
		self.commands.append(cmd)

if __name__ == '__main__':
	fj = Fujiyama()
	fj.OpenPlugin('PlasticShader.so')
	fj.SaveFrameBuffer('fb1', 'test.fb')
	fj.Run()

