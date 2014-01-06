#!/usr/bin/env python

# 1 cube fog volume and bunny with 1 point light
# NOTE this scene renders volumes on ONLY v.0.0.3
# On v.0.0.4 or higher, it renders floor and bunny
# though there is still a empty volume primitive.
# Copyright (c) 2011-2014 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('PlasticShader')
si.OpenPlugin('VolumeShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 5, .2, 5)
si.SetProperty3('cam1', 'rotate', -4.0446912353862681, 45, 0)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', 5, 12, -5)

#Shader
si.NewShader('bunny_shader', 'PlasticShader')
si.SetProperty3('bunny_shader', 'diffuse', .9, .6, .4)
si.SetProperty3('bunny_shader', 'reflect', 0, 0, 0)

si.NewShader('floor_shader', 'PlasticShader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)
si.SetProperty1('floor_shader', 'ior', 2)
si.NewShader('volume_shader1', 'VolumeShader')

#Volume
si.NewVolume('volume_data')

#Mesh
si.NewMesh('bunny_mesh', '../../mesh/bunny.mesh')
si.NewMesh('floor_mesh', '../../mesh/floor.mesh')

#ObjectInstance
si.NewObjectInstance('obj1', 'bunny_mesh')
si.AssignShader('obj1', 'bunny_shader')

si.NewObjectInstance('floor1', 'floor_mesh')
si.SetProperty3('floor1', 'translate', 3, -1.5, 3)
si.AssignShader('floor1', 'floor_shader')

si.NewObjectInstance('volume1', 'volume_data')
si.AssignShader('volume1', 'volume_shader1')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty2('ren1', 'resolution', 640, 480)

#SetProperty1 ren1 cast_shadow 0
si.SetProperty1('ren1', 'raymarch_step', .01)
si.SetProperty1('ren1', 'raymarch_shadow_step', .01)
si.SetProperty1('ren1', 'raymarch_reflect_step', .05)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../volume_and_bunny.fb')

#Run commands
si.Run()
#si.Print()
