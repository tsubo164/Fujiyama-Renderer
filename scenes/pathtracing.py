#!/usr/bin/env python

# 2 teapots and 2 bunnies with 1 sphere light
# Copyright (c) 2011-2016 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugin
si.OpenPlugin('pathtracing_shader', 'PathtracingShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 0.5, 1.85, 0)
si.SetProperty1('cam1', 'fov', 40)

#Light

#Shader
si.NewShader('floor_shader1', 'pathtracing_shader')
si.SetProperty3('floor_shader1', 'diffuse', .8, .8, .8)

si.NewShader('ceiling_shader1', 'pathtracing_shader')
si.SetProperty3('ceiling_shader1', 'diffuse', .8, .8, .8)

si.NewShader('wall_shader1', 'pathtracing_shader')
si.SetProperty3('wall_shader1', 'diffuse', .8, 0, 0)

si.NewShader('wall_shader2', 'pathtracing_shader')
si.SetProperty3('wall_shader2', 'diffuse', 0, .8, 0)

si.NewShader('wall_shader3', 'pathtracing_shader')
si.SetProperty3('wall_shader3', 'diffuse', .8, .8, .8)

si.NewShader('mirror_shader1', 'pathtracing_shader')
si.SetProperty3('mirror_shader1', 'diffuse', .05, .05, .05)
si.SetProperty3('mirror_shader1', 'reflect', 1, 1, .3)
si.SetProperty1('mirror_shader1', 'ior', 20)

si.NewShader('plastic_shader1', 'pathtracing_shader')
si.SetProperty3('plastic_shader1', 'diffuse', .2, .4, .8)
si.SetProperty3('plastic_shader1', 'reflect', 1, 1, 1)

si.NewShader('glass_shader1', 'pathtracing_shader')
si.SetProperty3('glass_shader1', 'diffuse', 0, 0, 0)
si.SetProperty3('glass_shader1', 'reflect', 1, 1, 1)
si.SetProperty3('glass_shader1', 'refract', 1, 1, 1)
si.SetProperty3('glass_shader1', 'filter_color', .2, .1, .0)

si.NewShader('light_shader1', 'pathtracing_shader')
si.SetProperty3('light_shader1', 'diffuse', 0, 0, 0)
si.SetProperty3('light_shader1', 'emission', 1, 1, 1)
si.SetProperty3('light_shader1', 'emission', 3, 3, 3)
si.SetProperty3('light_shader1', 'emission', 6, 6, 6)
emit_r = 1.2
emit_g = 1.1
emit_b = 0.9
emit_x = 12 + 8
si.SetProperty3('light_shader1', 'emission', emit_r*emit_x, emit_g*emit_x, emit_b*emit_x)

#Mesh
si.NewMesh('happy_mesh', '../../ply/happy.ply')
si.NewMesh('bunny_mesh', '../../ply/bunny.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')
si.NewMesh('sphere_mesh', '../../ply/sphere.ply')

#ObjectInstance
si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader1')
si.SetProperty3('floor1', 'scale', .1, .1, .1)

si.NewObjectInstance('ceiling1', 'floor_mesh')
si.AssignShader('ceiling1', 'DEFAULT_SHADING_GROUP', 'ceiling_shader1')
si.SetProperty3('ceiling1', 'scale', .1, .1, .1)
si.SetProperty3('ceiling1', 'rotate', 0, 0, 180)
si.SetProperty3('ceiling1', 'translate', 0, 1, 0)

si.NewObjectInstance('wall1', 'floor_mesh')
si.AssignShader('wall1', 'DEFAULT_SHADING_GROUP', 'wall_shader1')
si.SetProperty3('wall1', 'scale', .1, .1, .1)
si.SetProperty3('wall1', 'rotate', 0, 0, -90)
si.SetProperty3('wall1', 'translate', -.5, .5, 0)

si.NewObjectInstance('wall2', 'floor_mesh')
si.AssignShader('wall2', 'DEFAULT_SHADING_GROUP', 'wall_shader2')
si.SetProperty3('wall2', 'scale', .1, .1, .1)
si.SetProperty3('wall2', 'rotate', 0, 0, 90)
si.SetProperty3('wall2', 'translate', .5, .5, 0)

si.NewObjectInstance('wall3', 'floor_mesh')
si.AssignShader('wall3', 'DEFAULT_SHADING_GROUP', 'wall_shader3')
si.SetProperty3('wall3', 'scale', .1, .1, .1)
si.SetProperty3('wall3', 'rotate', 90, 0, 0)
si.SetProperty3('wall3', 'translate', 0, .5, -.5)

si.NewObjectInstance('bunny1', 'bunny_mesh')
#si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'mirror_shader1')
si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'glass_shader1')
si.SetProperty3('bunny1', 'translate', -.21, 0, .21)
si.SetProperty3('bunny1', 'scale', .3, .3, .3)

si.NewObjectInstance('sphere2', 'sphere_mesh')
si.AssignShader('sphere2', 'DEFAULT_SHADING_GROUP', 'glass_shader1')
si.SetProperty3('sphere2', 'translate', .25, .15, .15)
si.SetProperty3('sphere2', 'scale', .15, .15, .15)

si.NewObjectInstance('happy1', 'happy_mesh')
si.AssignShader('happy1', 'DEFAULT_SHADING_GROUP', 'plastic_shader1')
si.SetProperty3('happy1', 'translate', .0, 0, .0)
si.SetProperty3('happy1', 'scale', .25, .25, .25)

si.NewObjectInstance('light_source1', 'sphere_mesh')
si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
si.SetProperty3('light_source1', 'translate', 0, 1, 0)
si.SetProperty3('light_source1', 'scale', .2, .05, .2)
"""
si.NewObjectInstance('light_source1', 'floor_mesh')
si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
si.SetProperty3('light_source1', 'translate', 0, 1-.001, 0)
scale = .03
si.SetProperty3('light_source1', 'scale', scale, scale, scale)
"""

#ObjectGroup
#si.NewObjectGroup('group1')
#si.AddObjectToGroup('group1', 'teapot1')
#si.AddObjectToGroup('group1', 'teapot1')
#si.AddObjectToGroup('group1', 'teapot2')
#si.AddObjectToGroup('group1', 'bunny1')
#si.AddObjectToGroup('group1', 'bunny2')
#si.AssignObjectGroup('teapot1', 'shadow_target', 'group1')
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
si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty2('ren1', 'resolution', 160*2, 120*2)
si.SetProperty2('ren1', 'pixelsamples', 12, 12)
si.SetProperty2('ren1', 'pixelsamples', 24, 24)
si.SetProperty2('ren1', 'pixelsamples', 64, 64)
#TODO Fix adaptive_max_subdivision bug
#si.SetProperty1('ren1', 'sampler_type', 1)
#si.SetProperty1('ren1', 'adaptive_max_subdivision', 1)
#si.SetProperty1('ren1', 'max_reflect_depth', 5)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../pathtracing.fb')

#Run commands
si.Run()
#si.Print()
