#!/usr/bin/env python

# 4 head models and sss shaders with 1 point light
# Copyright (c) 2011-2016 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('SSSShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, .55, 2)
si.SetProperty3('cam1', 'rotate', -10, 0, 0)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', -5, 2, -2)

#Shader
si.NewShader('head_shader1', 'SSSShader')
si.SetProperty3('head_shader1', 'reflect', 0, 0, 0)
si.SetProperty1('head_shader1', 'enable_single_scattering', 1)
si.SetProperty1('head_shader1', 'enable_multiple_scattering', 0)
si.SetProperty1('head_shader1', 'single_scattering_samples', 1)
si.SetProperty1('head_shader1', 'multiple_scattering_samples', 1)
si.SetProperty3('head_shader1', 'scattering_coefficient', .007, .018, .003)
si.SetProperty3('head_shader1', 'absorption_coefficient', .097, .0061, .145)

si.NewShader('head_shader2', 'SSSShader')
si.SetProperty3('head_shader2', 'reflect', 0, 0, 0)
si.SetProperty1('head_shader2', 'enable_single_scattering', 1)
si.SetProperty1('head_shader2', 'enable_multiple_scattering', 0)
si.SetProperty1('head_shader2', 'single_scattering_samples', 1)
si.SetProperty1('head_shader2', 'multiple_scattering_samples', 1)
si.SetProperty3('head_shader2', 'scattering_coefficient', .07, .122, .19)
si.SetProperty3('head_shader2', 'absorption_coefficient', .00014, .00025, .00142)
si.SetProperty3('head_shader2', 'specular', .01, .01, .01)

si.NewShader('head_shader3', 'SSSShader')
si.SetProperty3('head_shader3', 'reflect', 0, 0, 0)
si.SetProperty1('head_shader3', 'enable_single_scattering', 1)
si.SetProperty1('head_shader3', 'enable_multiple_scattering', 0)
si.SetProperty1('head_shader3', 'single_scattering_samples', 1)
si.SetProperty1('head_shader3', 'multiple_scattering_samples', 1)
si.SetProperty3('head_shader3', 'scattering_coefficient', .074, .088, .101)
si.SetProperty3('head_shader3', 'absorption_coefficient', .0032, .017, .048)
si.SetProperty3('head_shader3', 'specular', .1, .1, .1)
si.SetProperty1('head_shader3', 'roughness', .2)

si.NewShader('head_shader4', 'SSSShader')
si.SetProperty3('head_shader4', 'reflect', 0, 0, 0)
si.SetProperty1('head_shader4', 'enable_single_scattering', 1)
si.SetProperty1('head_shader4', 'enable_multiple_scattering', 0)
si.SetProperty1('head_shader4', 'single_scattering_samples', 1)
si.SetProperty1('head_shader4', 'multiple_scattering_samples', 1)
si.SetProperty3('head_shader4', 'scattering_coefficient', .255, .321, .377)
si.SetProperty3('head_shader4', 'absorption_coefficient', .00011, .00024, .0014)

#Mesh
si.NewMesh('head_mesh', '../../obj/head.obj')

#ObjectInstance
si.NewObjectInstance('head1', 'head_mesh')
si.AssignShader('head1', 'DEFAULT_SHADING_GROUP', 'head_shader1')
si.SetProperty3('head1', 'translate', -.3, .5, 0)

si.NewObjectInstance('head2', 'head_mesh')
si.AssignShader('head2', 'DEFAULT_SHADING_GROUP', 'head_shader2')
si.SetProperty3('head2', 'translate', .3, .5, 0)

si.NewObjectInstance('head3', 'head_mesh')
si.AssignShader('head3', 'DEFAULT_SHADING_GROUP', 'head_shader3')
si.SetProperty3('head3', 'translate', -.3, 0, 0)

si.NewObjectInstance('head4', 'head_mesh')
si.AssignShader('head4', 'DEFAULT_SHADING_GROUP', 'head_shader4')
si.SetProperty3('head4', 'translate', .3, 0, 0)

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'head1')
si.AssignObjectGroup('head1', 'shadow_target', 'group1')

si.NewObjectGroup('group2')
si.AddObjectToGroup('group2', 'head2')
si.AssignObjectGroup('head2', 'shadow_target', 'group2')

si.NewObjectGroup('group3')
si.AddObjectToGroup('group3', 'head3')
si.AssignObjectGroup('head3', 'shadow_target', 'group3')

si.NewObjectGroup('group4')
si.AddObjectToGroup('group4', 'head4')
si.AssignObjectGroup('head4', 'shadow_target', 'group4')

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
si.SaveFrameBuffer('fb1', '../subsurface_scattering.fb')

#Run commands
si.Run()
#si.Print()

