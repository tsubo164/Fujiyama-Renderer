#!/usr/bin/env python

# 2 teapots and 2 bunnies with 1 sphere light
# Copyright (c) 2011-2020 Hiroshi Tsubokawa

import fujiyama
import math

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('material_shader', 'MaterialShader')
#si.OpenPlugin('pathtracing_shader', 'PlasticShader')
si.OpenPlugin('stanfordply_procedure', 'StanfordPlyProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 2, 8)
si.SetProperty3('cam1', 'rotate', -15, 0, 0)

'''
#Light
si.NewLight('light1', 'SphereLight')
si.SetProperty3('light1', 'translate', 0, 2, 0)
#si.SetProperty3('light1', 'translate', 0, 1, 2)
si.SetProperty3('light1', 'scale', .5, .5, .5)
si.SetProperty1('light1', 'intensity', 5)

si.SetProperty1('light1', 'sample_count', 16)
'''

#Shader
'''
si.NewShader('teapot_shader', 'material_shader')
si.SetProperty3('teapot_shader', 'reflect', .0, .0, .0)

si.NewShader('bunny_shader', 'material_shader')
si.SetProperty3('bunny_shader', 'reflect', .0, .0, .0)
'''

si.NewShader('floor_shader', 'material_shader')
#si.SetProperty3('floor_shader', 'reflect', .0, .0, .0)
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)

intensity = 15
si.NewShader('sphere_shader', 'constant_shader')
si.SetProperty3('sphere_shader', 'diffuse', intensity, intensity, intensity)

si.NewShader('light_shader1', 'material_shader')
si.SetProperty3('light_shader1', 'diffuse', 0, 0, 0)
emit_r = 1.2
emit_g = 1.1
emit_b = 0.9
emit_x = 20
#NOTE When using CG light for direct lighting (not yet)
'''
emit_x = 10
'''
si.SetProperty3('light_shader1', 'emission', emit_r*emit_x, emit_g*emit_x, emit_b*emit_x)

#Mesh
#si.NewMesh('teapot_mesh')
#si.NewMesh('bunny_mesh')
si.NewMesh('floor_mesh')
si.NewMesh('sphere_mesh')

#Procedure
#si.NewProcedure('teapot_proc', 'stanfordply_procedure')
#si.AssignMesh('teapot_proc', 'mesh', 'teapot_mesh')
#si.SetStringProperty('teapot_proc', 'filepath', '../../ply/teapot.ply')
#si.SetStringProperty('teapot_proc', 'io_mode', 'r')
#si.RunProcedure('teapot_proc')

#si.NewProcedure('bunny_proc', 'stanfordply_procedure')
#si.AssignMesh('bunny_proc', 'mesh', 'bunny_mesh')
#si.SetStringProperty('bunny_proc', 'filepath', '../../ply/bunny.ply')
#si.SetStringProperty('bunny_proc', 'io_mode', 'r')
#si.RunProcedure('bunny_proc')

si.NewProcedure('floor_proc', 'stanfordply_procedure')
si.AssignMesh('floor_proc', 'mesh', 'floor_mesh')
si.SetStringProperty('floor_proc', 'filepath', '../../ply/floor.ply')
si.SetStringProperty('floor_proc', 'io_mode', 'r')
si.RunProcedure('floor_proc')

si.NewProcedure('sphere_proc', 'stanfordply_procedure')
si.AssignMesh('sphere_proc', 'mesh', 'sphere_mesh')
si.SetStringProperty('sphere_proc', 'filepath', '../../ply/sphere.ply')
si.SetStringProperty('sphere_proc', 'io_mode', 'r')
si.RunProcedure('sphere_proc')

#ObjectInstance
distance = 1.5
#si.NewObjectInstance('teapot1', 'teapot_mesh')
#si.SetProperty3('teapot1', 'translate', -distance, 0, distance)
#si.AssignShader('teapot1', 'DEFAULT_SHADING_GROUP', 'teapot_shader')

#si.NewObjectInstance('teapot2', 'teapot_mesh')
#si.SetProperty3('teapot2', 'translate', distance, 0, -distance)
#si.AssignShader('teapot2', 'DEFAULT_SHADING_GROUP', 'teapot_shader')

#si.NewObjectInstance('bunny1', 'bunny_mesh')
#si.SetProperty3('bunny1', 'translate', distance, 0, distance)
#si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'bunny_shader')

#si.NewObjectInstance('bunny2', 'bunny_mesh')
#si.SetProperty3('bunny2', 'translate', -distance, 0, -distance)
#si.AssignShader('bunny2', 'DEFAULT_SHADING_GROUP', 'bunny_shader')

'''
si.NewObjectInstance('floor1', 'floor_mesh')
si.SetProperty3('floor1', 'translate', 0, 0, -500)
si.SetProperty3('floor1', 'scale', 200, 1, 200)
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')
'''

'''
radius = 3 + 4
offsety = .05
offsetz = 3.5
angle = 11 - 5

for i in range(5):
    ty = -radius * math.cos(math.radians(angle * i)) + offsety + radius
    tz = -radius * math.sin(math.radians(angle * i)) + offsetz
    rx = angle * i
    name = 'grid' + str(i)
    si.NewObjectInstance(name, 'floor_mesh')
    si.SetProperty3(name, 'translate', 0, ty, tz)
    si.SetProperty3(name, 'scale', .3, 1, .05)
    si.SetProperty3(name, 'rotate', rx, 0, 0)
    si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'floor_shader')
'''

#grids
name = 'grid1'
si.NewObjectInstance(name, 'floor_mesh')
si.SetProperty3(name, 'translate', 0, .1, 3.8)
si.SetProperty3(name, 'scale', .3, 1, .07)
si.SetProperty3(name, 'rotate', 0, 0, 0)
si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'floor_shader')

name = 'grid2'
si.NewObjectInstance(name, 'floor_mesh')
si.SetProperty3(name, 'translate', 0, .1+.05, 3.8 - .8)
si.SetProperty3(name, 'scale', .3, 1, .07)
si.SetProperty3(name, 'rotate', 6, 0, 0)
si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'floor_shader')

name = 'grid3'
si.NewObjectInstance(name, 'floor_mesh')
si.SetProperty3(name, 'translate', 0, .1+.18, 3.8 - 1.6)
si.SetProperty3(name, 'scale', .3, 1, .07)
si.SetProperty3(name, 'rotate', 14.5, 0, 0)
si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'floor_shader')

name = 'grid4'
si.NewObjectInstance(name, 'floor_mesh')
si.SetProperty3(name, 'translate', 0, .1+.45, 3.8 - 2.4)
si.SetProperty3(name, 'scale', .3, 1, .07)
si.SetProperty3(name, 'rotate', 30, 0, 0)
si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'floor_shader')
'''
'''

'''
si.NewObjectInstance('sphere1', 'sphere_mesh')
si.AssignShader('sphere1', 'DEFAULT_SHADING_GROUP', 'sphere_shader')
si.SetProperty3('sphere1', 'translate', 0, 2, 0)
#si.SetProperty3('sphere1', 'translate', 0, 1, 2)
si.SetProperty3('sphere1', 'scale', .5, .5, .5)
'''

width = 2.5 + .2

for i in range(4-3):
    tx = .5 * width - width/3. * i
    ty = 1.3
    tz = 1.2
    s = .3 * pow(.5,i)
    name = 'light_geo' + str(i)
    si.NewObjectInstance(name, 'sphere_mesh')
    si.SetProperty3(name, 'translate', tx, ty, tz)
    si.SetProperty3(name, 'scale', s, s, s)
    si.AssignShader(name, 'DEFAULT_SHADING_GROUP', 'light_shader1')

    name = 'light' + str(i)
    si.NewLight(name, 'SphereLight')
    si.SetProperty3(name, 'translate', tx, ty, tz)
    si.SetProperty3(name, 'scale', s, s, s)
    si.SetProperty1(name, 'intensity', 5)

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'grid1')
#si.AddObjectToGroup('group1', 'teapot2')
#si.AddObjectToGroup('group1', 'bunny1')
#si.AddObjectToGroup('group1', 'bunny2')
si.AssignObjectGroup('grid1', 'shadow_target', 'group1')
#si.AssignObjectGroup('teapot2', 'shadow_target', 'group1')
#si.AssignObjectGroup('bunny1', 'shadow_target', 'group1')
#si.AssignObjectGroup('bunny2', 'shadow_target', 'group1')
#si.AssignObjectGroup('floor1', 'shadow_target', 'group1')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty2('ren1', 'pixelsamples', 12, 12)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../mis.fb')

#Run commands
si.Run()
#si.Print()
