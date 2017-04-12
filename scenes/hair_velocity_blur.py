#!/usr/bin/env python

# 1 hair curve and 1 head object with 1 dome light with an HDRI
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

# NOTE How to make hair.crv
# $ obj2mesh head.obj  head.mesh
# $ curvegen --hair head.mesh hair.crv

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('hair_shader', 'HairShader')
si.OpenPlugin('curve_generator_procedure', 'CurveGeneratorProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', .08, -.05, 1.5)

obj_rot = -90
rot = 110

#Light
si.NewLight('light1', 'DomeLight')
si.SetProperty3('light1', 'rotate', 0, rot, 0)
#si.SetProperty1('light1', 'sample_count', 256)
si.SetProperty1('light1', 'sample_count', 32)

#Texture
si.NewTexture('tex1', '../../hdr/pisa.hdr')
si.AssignTexture('light1', 'environment_map', 'tex1');

#Shader
si.NewShader('head_shader', 'plastic_shader')
si.SetProperty3('head_shader', 'diffuse', .8, .8, .8)

si.NewShader('hair_shader1', 'hair_shader')

si.NewShader('dome_shader', 'constant_shader')
si.AssignTexture('dome_shader', 'texture', 'tex1')

#Curve
si.NewCurve('curve_data')

#Mesh
si.NewMesh('head_mesh', '../../obj/head.obj')
si.NewMesh('dome_mesh', '../../ply/dome.ply')

#Procedure
si.NewProcedure('proc1', 'curve_generator_procedure')
si.AssignMesh('proc1', 'mesh', 'head_mesh')
si.AssignCurve('proc1', 'curve', 'curve_data')
si.SetProperty1('proc1', 'is_hair', 1)
si.RunProcedure('proc1')

#ObjectInstance
si.NewObjectInstance('head1', 'head_mesh')
si.SetProperty3('head1', 'rotate', 0, obj_rot, 0)
si.AssignShader('head1', 'DEFAULT_SHADING_GROUP', 'head_shader')

si.NewObjectInstance('hair1', 'curve_data')
si.SetProperty3('hair1', 'rotate', 0, obj_rot, 0)
si.AssignShader('hair1', 'DEFAULT_SHADING_GROUP', 'hair_shader1')

si.NewObjectInstance('dome1', 'dome_mesh')
si.SetProperty3('dome1', 'rotate', 0, rot, 0)
si.SetProperty3('dome1', 'scale', -.5, .5, .5)
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

#ObjectGroup
# Create shadow_target for some objects.
# Since 'DomeLight' has infinite distance, we need to exclude
# 'dome1' object which is for just background image.
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'head1')
si.AddObjectToGroup('group1', 'hair1')
si.AssignObjectGroup('hair1', 'shadow_target', 'group1')
si.AssignObjectGroup('head1', 'shadow_target', 'group1')

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
si.SaveFrameBuffer('fb1', '../hair_velocity_blur.fb')

#Run commands
si.Run()
#si.Print()
