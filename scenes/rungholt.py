#!/usr/bin/env python

# 1 rungholt with 1 dome light with an HDRI
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama
import wavefrontobj

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('wavefrontobj_procedure', 'WavefrontObjProcedure')
si.OpenPlugin('stanfordply_procedure', 'StanfordPlyProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 1, 3)
si.SetProperty3('cam1', 'rotate', -35, 0, 0)
si.SetProperty1('cam1', 'fov', 80)

rot = 190

#Light
si.NewLight('light1', 'DomeLight')
si.SetProperty3('light1', 'rotate', 0, rot, 0)
# 'sample_count' property defines how many samples we take from hdr.
# More samples produces more acculate and less noisy images
si.SetProperty1('light1', 'sample_count', 128)

#Texture
si.NewTexture('tex1', '../../hdr/austria.hdr')
si.AssignTexture('light1', 'environment_map', 'tex1')

#Shader
si.NewShader('dome_shader', 'constant_shader')
si.AssignTexture('dome_shader', 'texture', 'tex1')

#Mesh
si.NewMesh('rungholt_mesh', 'null')
si.NewMesh('dome_mesh',   'null')

#Procedure
si.NewProcedure('dome_proc', 'stanfordply_procedure')
si.AssignMesh('dome_proc', 'mesh', 'dome_mesh')
si.SetStringProperty('dome_proc', 'filepath', '../../ply/dome.ply')
si.SetStringProperty('dome_proc', 'io_mode', 'r')
si.RunProcedure('dome_proc')

si.NewProcedure('rungholt_proc', 'wavefrontobj_procedure')
si.AssignMesh('rungholt_proc', 'mesh', 'rungholt_mesh')
si.SetStringProperty('rungholt_proc', 'filepath', '../../obj/rungholt/rungholt.obj')
si.SetStringProperty('rungholt_proc', 'io_mode', 'r')
si.RunProcedure('rungholt_proc')

#ObjectInstance
si.NewObjectInstance('rungholt1', 'rungholt_mesh')
si.SetProperty3('rungholt1', 'scale', .01, .01, .01)

si.NewObjectInstance('dome1', 'dome_mesh')
si.SetProperty3('dome1', 'rotate', 0, rot, 0)
si.SetProperty3('dome1', 'scale', -.5, .5, .5)
# DEFAULT_SHADING_GROUP represents all primitives
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

#Material from *.obj
# This function calls fujiyama scene interfaces such as
# NewShader, SetProperty* and AssignShader.
# It parses *.obj to find face group names and material names.
# And it also parses *.mtl to set up shaders.
wavefrontobj.assign_materials(si, '../../obj/rungholt/rungholt.obj', 'rungholt1', 'plastic_shader')

#ObjectGroup
# Create shadow_target for some objects.
# Since 'DomeLight' has infinite distance, we need to exclude
# 'dome1' object which is for just background image.
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'rungholt1')
si.AssignObjectGroup('rungholt1', 'shadow_target', 'group1')

si.NewObjectGroup('group2')
si.AddObjectToGroup('group2', 'dome1')
si.AddObjectToGroup('group2', 'rungholt1')
si.AssignObjectGroup('rungholt1', 'reflect_target', 'group2')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../rungholt.fb')

#Run commands
si.Run()
#si.Print()
