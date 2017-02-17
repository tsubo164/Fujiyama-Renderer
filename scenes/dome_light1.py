#!/usr/bin/env python

# 1 happy with 1 dome light with an HDRI
# Copyright (c) 2011-2016 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 1, 8.5, 0)

rot = 110

#Light
si.NewLight('light1', 'DomeLight')
si.SetProperty3('light1', 'rotate', 0, rot, 0)
si.SetProperty1('light1', 'sample_count', 256)

#Texture
si.NewTexture('tex1', '../../hdr/pisa.hdr')
si.AssignTexture('light1', 'environment_map', 'tex1');

#Shader
si.NewShader('happy_shader', 'plastic_shader')
si.SetProperty3('happy_shader', 'diffuse', .8, .8, .8)

si.NewShader('dome_shader', 'constant_shader')
si.AssignTexture('dome_shader', 'texture', 'tex1')

si.NewShader('sphere_shader1', 'plastic_shader')
si.SetProperty3('sphere_shader1', 'diffuse', 0, 0, 0)
si.SetProperty1('sphere_shader1', 'ior', 40)

si.NewShader('sphere_shader2', 'plastic_shader')
si.SetProperty3('sphere_shader2', 'diffuse', .5, .5, .5)
si.SetProperty3('sphere_shader2', 'reflect', 0, 0, 0)

#Mesh
si.NewMesh('happy_mesh', '../../ply/happy.ply')
si.NewMesh('dome_mesh', '../../ply/dome.ply')
si.NewMesh('sphere_mesh', '../../ply/sphere.ply')

#ObjectInstance
si.NewObjectInstance('happy1', 'happy_mesh')
si.AssignShader('happy1', 'DEFAULT_SHADING_GROUP', 'happy_shader')

si.NewObjectInstance('dome1', 'dome_mesh')
si.SetProperty3('dome1', 'rotate', 0, rot, 0)
si.SetProperty3('dome1', 'scale', -.5, .5, .5)
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

si.NewObjectInstance('sphere1', 'sphere_mesh')
si.AssignShader('sphere1', 'DEFAULT_SHADING_GROUP', 'sphere_shader1')
si.SetProperty3('sphere1', 'translate', -1.5, -.5, 0)
si.SetProperty3('sphere1', 'scale', .5, .5, .5)

si.NewObjectInstance('sphere2', 'sphere_mesh')
si.AssignShader('sphere2', 'DEFAULT_SHADING_GROUP', 'sphere_shader2')
si.SetProperty3('sphere2', 'translate', 1.5, -.5, 0)
si.SetProperty3('sphere2', 'scale', .5, .5, .5)

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'happy1')
si.AssignObjectGroup('happy1', 'shadow_target', 'group1')

si.NewObjectGroup('group2')
si.AssignObjectGroup('sphere1', 'shadow_target', 'group2')
si.AssignObjectGroup('sphere2', 'shadow_target', 'group2')

si.NewObjectGroup('group3')
si.AddObjectToGroup('group3', 'dome1')
si.AssignObjectGroup('sphere1', 'reflect_target', 'group3')

si.NewObjectGroup('group4')
si.AddObjectToGroup('group4', 'dome1')
si.AddObjectToGroup('group4', 'happy1')
si.AssignObjectGroup('happy1', 'reflect_target', 'group4')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
#si.SetProperty2('ren1', 'pixelsamples', 9, 9)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../dome_light1.fb')

#Run commands
si.Run()
#si.Print()

