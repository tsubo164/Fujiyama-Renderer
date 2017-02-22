#!/usr/bin/env python

# 2 teapots and 2 bunnies with 1 sphere light
# Copyright (c) 2011-2016 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('pathtracing_shader', 'PathtracingShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 0.5, 1.85, 0)
#si.SetSampleProperty3('cam1', 'rotate', -45, 0, 0, 0)
si.SetProperty1('cam1', 'fov', 40)

#Light
#si.NewLight('light1', 'SphereLight')
#si.NewLight('light1', 'PointLight')
#si.SetProperty3('light1', 'translate', 0, 2, 0)
#si.SetProperty3('light1', 'scale', .5, .5, .5)
#si.SetProperty1('light1', 'intensity', 5)
#si.SetProperty1('light1', 'sample_count', 16)

#Shader
#si.NewShader('teapot_shader', 'plastic_shader')
#si.SetProperty3('teapot_shader', 'reflect', .0, .0, .0)

#si.NewShader('bunny_shader', 'plastic_shader')
#si.SetProperty3('bunny_shader', 'reflect', .0, .0, .0)

si.NewShader('floor_shader1', 'pathtracing_shader')
si.SetProperty3('floor_shader1', 'diffuse', .8, .8, .8)

si.NewShader('ceiling_shader1', 'pathtracing_shader')
si.SetProperty3('ceiling_shader1', 'diffuse', .8, .8, .8)

si.NewShader('wall_shader1', 'pathtracing_shader')
si.SetProperty3('wall_shader1', 'diffuse', .5, 0, 0)

si.NewShader('wall_shader2', 'pathtracing_shader')
si.SetProperty3('wall_shader2', 'diffuse', 0, .5, 0)

si.NewShader('wall_shader3', 'pathtracing_shader')
si.SetProperty3('wall_shader3', 'diffuse', .8, .8, .8)

si.NewShader('mirror_shader1', 'pathtracing_shader')
si.SetProperty3('mirror_shader1', 'reflect', 1, 1, .3)

si.NewShader('glass_shader1', 'pathtracing_shader')
si.SetProperty3('glass_shader1', 'reflect', 1, 1, 1)
si.SetProperty3('glass_shader1', 'refract', 1, 1, 1)

si.NewShader('light_shader1', 'pathtracing_shader')
si.SetProperty3('light_shader1', 'emission', 1, 1, 1)
si.SetProperty3('light_shader1', 'emission', 3, 3, 3)
si.SetProperty3('light_shader1', 'emission', 6, 6, 6)

#Mesh
#si.NewMesh('teapot_mesh', '../../ply/teapot.ply')
si.NewMesh('bunny_mesh', '../../ply/bunny.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')
si.NewMesh('sphere_mesh', '../../ply/sphere.ply')

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
si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'mirror_shader1')
si.SetProperty3('bunny1', 'translate', -.21, 0, .21)
si.SetProperty3('bunny1', 'scale', .3, .3, .3)

si.NewObjectInstance('sphere2', 'sphere_mesh')
si.AssignShader('sphere2', 'DEFAULT_SHADING_GROUP', 'glass_shader1')
si.SetProperty3('sphere2', 'translate', .25, .15, .15)
si.SetProperty3('sphere2', 'scale', .15, .15, .15)

"""
si.NewObjectInstance('light_source1', 'sphere_mesh')
si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
si.SetProperty3('light_source1', 'translate', 0, 1, 0)
si.SetProperty3('light_source1', 'scale', .2, .2, .2)
"""
si.NewObjectInstance('light_source1', 'floor_mesh')
si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
si.SetProperty3('light_source1', 'translate', 0, 1-.001, 0)
scale = .03
si.SetProperty3('light_source1', 'scale', scale, scale, scale)

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
#si.SetProperty1('ren1', 'max_refract_depth', 20)
#si.SetProperty2('ren1', 'pixelsamples', 2, 2)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../pathtracing.fb')

#Run commands
si.Run()
#si.Print()
