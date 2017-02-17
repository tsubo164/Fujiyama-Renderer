#!/usr/bin/env python

# 1 cube fog volume and bunny with 1 point light
# Copyright (c) 2011-2016 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('volume_shader', 'VolumeShader')
si.OpenPlugin('ConstantVolumeProcedure', 'ConstantVolumeProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 5, .2, 5)
si.SetProperty3('cam1', 'rotate', -4.0446912353862681, 45, 0)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', 5, 12, -5)

#Shader
si.NewShader('bunny_shader', 'plastic_shader')
si.SetProperty3('bunny_shader', 'diffuse', .9, .6, .4)
si.SetProperty3('bunny_shader', 'reflect', 0, 0, 0)

si.NewShader('floor_shader', 'plastic_shader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)
si.SetProperty1('floor_shader', 'ior', 2)
si.NewShader('volume_shader1', 'volume_shader')

#Volume
si.NewVolume('volume_data')
si.SetProperty3('volume_data', 'bounds_min', -1, -1, -1)
si.SetProperty3('volume_data', 'bounds_max', 1, 1, 1)
si.SetProperty3('volume_data', 'resolution', 100, 100, 100)

#Procedure
si.NewProcedure('proc1', 'ConstantVolumeProcedure')
si.AssignVolume('proc1', 'volume', 'volume_data')
si.SetProperty1('proc1', 'density', 1.)
si.RunProcedure('proc1')

#Mesh
si.NewMesh('bunny_mesh', '../../ply/bunny.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')

#ObjectInstance
si.NewObjectInstance('bunny1', 'bunny_mesh')
si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'bunny_shader')

si.NewObjectInstance('floor1', 'floor_mesh')
si.SetProperty3('floor1', 'translate', 3, -1.5, 3)
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')

si.NewObjectInstance('volume1', 'volume_data')
si.AssignShader('volume1', 'DEFAULT_SHADING_GROUP', 'volume_shader1')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)

#SetProperty1 ren1 cast_shadow 0
si.SetProperty1('ren1', 'raymarch_step', .02)
si.SetProperty1('ren1', 'raymarch_shadow_step', .05)
si.SetProperty1('ren1', 'raymarch_reflect_step', .05)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../volume_and_bunny.fb')

#Run commands
si.Run()
#si.Print()
