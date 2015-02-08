#!/usr/bin/env python

import fujiyama
import os, sys

class Vec3:
	def __init__(self, x = 0, y = 0, z = 0):
		self.x = x
		self.y = y
		self.z = z

class Material:
	def __init__(self, name):
		self.name = name
		self.Ns = 10
		self.Ni = 1
		self.d  = 1
		self.Tr = 0
		self.Tf = Vec3(1, 1, 1)
		self.illum = 2
		self.Ka = Vec3(.8, .8, .8)
		self.Kd = Vec3(.8, .8, .8)
		self.Ks = Vec3(.8, .8, .8)
		self.Ke = Vec3(0, 0, 0)

def read_mtl(filename):
	mtl = None
	mtls = []
	for line in open(filename, 'r'):
		line = line.rstrip()
		tokens = line.split()

		if len(tokens) == 0:
			continue

		stmt = tokens[0]

		if stmt == 'newmtl':
			mtl = Material(tokens[1])
			mtls.append(mtl)
		elif stmt == 'Ns':
			mtl.Ns = float(tokens[1])
		elif stmt == 'Ni':
			mtl.Ni = float(tokens[1])
		elif stmt == 'd':
			mtl.d = float(tokens[1])
		elif stmt == 'Tr':
			mtl.Tr = float(tokens[1])
		elif stmt == 'Ka':
			mtl.Ka = Vec3(float(tokens[1]), float(tokens[2]), float(tokens[3]))
		elif stmt == 'Kd':
			mtl.Kd = Vec3(float(tokens[1]), float(tokens[2]), float(tokens[3]))
		elif stmt == 'Ks':
			mtl.Ks = Vec3(float(tokens[1]), float(tokens[2]), float(tokens[3]))

	return mtls

def read_obj(filename):
	all_mtls = []
	group_to_mtl = {}
	current_group = ''

	obj_dir = os.path.dirname(filename)

	for line in open(filename, 'r'):
		line = line.rstrip()
		tokens = line.split()

		if len(tokens) == 0:
			continue

		stmt = tokens[0]

		if stmt == 'mtllib':
			mtllib_name = tokens[1]
			#print 'mtllib', mtllib_name
			mtl_path = os.path.join(obj_dir, mtllib_name)
			print 'reading', mtl_path
			mtls = read_mtl(mtl_path)
			all_mtls.extend(mtls)
		elif stmt == 'usemtl':
			usemtl_name = tokens[1]
			group_to_mtl[current_group] = usemtl_name
		elif stmt == 'g':
			current_group = tokens[1]

	return (all_mtls, group_to_mtl)
	
def assign_materials(scene_interface, obj_filename, objinstance_name, shader_name):
	si = scene_interface
	all_mtls = []
	group_to_mtl = {}

	all_mtls, group_to_mtl = read_obj(obj_filename)

	for m in all_mtls:
		si.NewShader(m.name, shader_name)
		si.SetProperty3(m.name, 'ambient', m.Ka.x, m.Ka.y, m.Ka.z)
		si.SetProperty3(m.name, 'diffuse', m.Kd.x, m.Kd.y, m.Kd.z)
		si.SetProperty3(m.name, 'reflect', m.Ks.x, m.Ks.y, m.Ks.z)
	
	for g,m in group_to_mtl.items():
		si.AssignShader(objinstance_name, g, m)

def main():
	if len(sys.argv) != 2:
		print 'error'
		sys.exit()
	obj_filename = sys.argv[1]
	print 'reading', obj_filename

	all_mtls = []
	group_to_mtl = {}
	all_mtls, group_to_mtl = read_obj(obj_filename)
	print

	for m in all_mtls:
		print 'si.NewShader(%s, %s)' % (m.name, 'PlasticShader')
		print 'si.SetProperty3(%s, %s, %s, %s, %s)'%(m.name, 'ambient', m.Ka.x, m.Ka.y, m.Ka.z)
		print 'si.SetProperty3(%s, %s, %s, %s, %s)'%(m.name, 'diffuse', m.Kd.x, m.Kd.y, m.Kd.z)
		print 'si.SetProperty3(%s, %s, %s, %s, %s)'%(m.name, 'reflection', m.Ks.x, m.Ks.y, m.Ks.z)
		print
	print
	
	for g,m in group_to_mtl.items():
		print 'si.AssignShader(%s, %s, %s)' % ('myobj', g, m)
	print


if __name__ == '__main__':
	main()

