#!/usr/bin/env python

# 1 point cloud with 1 dome light with an HDRI
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('pointcloud_generator', 'PointcloudGenerator')

"""
#--------------
si.OpenPlugin('ConstantVolumeProcedure', 'ConstantVolumeProcedure')
#Volume
si.NewVolume('volume_data')
si.SetProperty3('volume_data', 'bounds_min', -1, -1, -1)
si.SetProperty3('volume_data', 'bounds_max', 1, 1, 1)
si.SetProperty3('volume_data', 'resolution', 100, 100, 100)
#Procedure
si.NewProcedure('proc2', 'ConstantVolumeProcedure')
si.AssignVolume('proc2', 'volume', 'volume_data')
si.SetProperty1('proc2', 'density', 1.)
si.RunProcedure('proc2')
#--------------
"""

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, .5, 3)
si.SetProperty3('cam1', 'rotate', -1, 0, 0)

#Light
rot = -30
si.NewLight('light1', 'DomeLight')
si.SetProperty3('light1', 'rotate', 0, rot, 0)
si.SetProperty1('light1', 'sample_count', 32)

#Texture
si.NewTexture('tex1', '../../hdr/austria.hdr')
si.AssignTexture('light1', 'environment_map', 'tex1');

#Shader
si.NewShader('floor_shader', 'plastic_shader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)

si.NewShader('ptc_shader1', 'plastic_shader')
si.SetProperty3('ptc_shader1', 'reflect', .0, .0, .0)

si.NewShader('dome_shader', 'constant_shader')
si.AssignTexture('dome_shader', 'texture', 'tex1')

#PointCloud
si.NewPointCloud('ptc_data', 'null')

#Mesh
si.NewMesh('bunny_mesh', '../../ply/bunny.ply')
si.NewMesh('dome_mesh', '../../ply/dome.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')

#Procedure
si.NewProcedure('proc1', 'pointcloud_generator')
si.AssignMesh('proc1', 'mesh', 'bunny_mesh')
si.AssignPointCloud('proc1', 'pointcloud', 'ptc_data')
si.RunProcedure('proc1')

#ObjectInstance
si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')

si.NewObjectInstance('point_cloud1', 'ptc_data')
si.AssignShader('point_cloud1', 'DEFAULT_SHADING_GROUP', 'ptc_shader1')

si.NewObjectInstance('dome1', 'dome_mesh')
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')
si.SetProperty3('dome1', 'rotate', 0, rot, 0)

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'point_cloud1')
si.AssignObjectGroup('point_cloud1', 'shadow_target', 'group1')
si.AssignObjectGroup('floor1', 'shadow_target', 'group1')

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
si.SaveFrameBuffer('fb1', '../point_cloud.fb')

#Run commands
si.Run()
#si.Print()
