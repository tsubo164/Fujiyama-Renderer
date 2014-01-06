#!/usr/bin/env python

# 1 dragon with 1 point light
# Copyright (c) 2011-2014 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('ConstantShader')
si.OpenPlugin('PlasticShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 2, 6, 0)
si.SetSampleProperty3('cam1', 'translate', 0, 2, 5.3, 1)
si.SetSampleProperty3('cam1', 'rotate', -10, 0, 0, 0)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', -10, 12, 10)

#Texture
si.NewTexture('tex1', '../../mip/ennis.mip')

#Shader
si.NewShader('dragon_shader', 'PlasticShader')
si.SetProperty3('dragon_shader', 'diffuse', .1, .4, .0)

si.NewShader('floor_shader', 'PlasticShader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)

si.NewShader('dome_shader', 'ConstantShader')
si.SetProperty3('dome_shader', 'diffuse', .8, .8, .8)
si.AssignTexture('dome_shader', 'texture', 'tex1')

#Mesh
si.NewMesh('dragon_mesh', '../../mesh/dragon.mesh')
si.NewMesh('floor_mesh', '../../mesh/floor.mesh')
si.NewMesh('dome_mesh', '../../mesh/dome.mesh')

#ObjectInstance
si.NewObjectInstance('dragon1', 'dragon_mesh')
si.AssignShader('dragon1', 'dragon_shader')

si.SetProperty3('dragon1', 'rotate', 0, -90, 0)
si.SetProperty3('dragon1', 'scale', .5, .5, .5)

si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'floor_shader')

si.NewObjectInstance('dome1', 'dome_mesh')
si.AssignShader('dome1', 'dome_shader')

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'dragon1')
si.AssignObjectGroup('dragon1', 'shadow_target', 'group1')
si.AssignObjectGroup('floor1', 'shadow_target', 'group1')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty2('ren1', 'pixelsamples', 9, 9)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../camera_motion_blur.fb')

#Run commands
si.Run()
#si.Print()

