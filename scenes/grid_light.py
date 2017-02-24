#!/usr/bin/env python

# 1 armadillo with 1 grid light
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 1.5, 8, 0)

#Light
si.NewLight('light1', 'GridLight')
si.SetProperty3('light1', 'translate', 0, 10, 0)
si.SetProperty3('light1', 'rotate', 0, 0, 180)
scale = 7
si.SetProperty3('light1', 'scale', scale, scale, scale)

si.SetProperty1('light1', 'sample_count', 8)

#Texture
si.NewTexture('tex1', '../../hdr/grace-new.hdr')

#Shader
si.NewShader('armadillo_shader', 'plastic_shader')
si.SetProperty3('armadillo_shader', 'diffuse', .0, .0, .0)
si.SetProperty1('armadillo_shader', 'ior', 10)

si.NewShader('floor_shader', 'plastic_shader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)
si.SetProperty3('floor_shader', 'reflect', 0, 0, 0)

si.NewShader('dome_shader', 'constant_shader')
si.AssignTexture('dome_shader', 'texture', 'tex1')

#Mesh
si.NewMesh('armadillo_mesh', '../../ply/armadillo.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')
si.NewMesh('dome_mesh', '../../ply/dome.ply')

#ObjectInstance
si.NewObjectInstance('armadillo1', 'armadillo_mesh')
si.AssignShader('armadillo1', 'DEFAULT_SHADING_GROUP', 'armadillo_shader')

si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')

si.NewObjectInstance('dome1', 'dome_mesh')
si.SetProperty3('dome1', 'rotate', 0, -90, 0)
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'armadillo1')
si.AssignObjectGroup('armadillo1', 'shadow_target', 'group1')
si.AssignObjectGroup('floor1', 'shadow_target', 'group1')

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
si.SaveFrameBuffer('fb1', '../grid_light.fb')

#Run commands
si.Run()
#si.Print()

