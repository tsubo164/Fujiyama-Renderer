#!/usr/bin/env python

# 1 point cloud with 1 dome light with an HDRI
# Copyright (c) 2011-2020 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('pointcloud_generator', 'PointcloudGenerator')
si.OpenPlugin('stanfordply_procedure', 'StanfordPlyProcedure')

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
si.NewPointCloud('ptc_data')

#Mesh
si.NewMesh('bunny_mesh')
si.NewMesh('floor_mesh')
si.NewMesh('dome_mesh')

#Procedure
si.NewProcedure('bunny_proc', 'stanfordply_procedure')
si.AssignMesh('bunny_proc', 'mesh', 'bunny_mesh')
si.SetStringProperty('bunny_proc', 'filepath', '../../ply/bunny.ply')
si.SetStringProperty('bunny_proc', 'io_mode', 'r')
si.RunProcedure('bunny_proc')

si.NewProcedure('floor_proc', 'stanfordply_procedure')
si.AssignMesh('floor_proc', 'mesh', 'floor_mesh')
si.SetStringProperty('floor_proc', 'filepath', '../../ply/floor.ply')
si.SetStringProperty('floor_proc', 'io_mode', 'r')
si.RunProcedure('floor_proc')

si.NewProcedure('dome_proc', 'stanfordply_procedure')
si.AssignMesh('dome_proc', 'mesh', 'dome_mesh')
si.SetStringProperty('dome_proc', 'filepath', '../../ply/dome.ply')
si.SetStringProperty('dome_proc', 'io_mode', 'r')
si.RunProcedure('dome_proc')

si.NewProcedure('ptc_proc', 'pointcloud_generator')
si.AssignMesh('ptc_proc', 'mesh', 'bunny_mesh')
si.AssignPointCloud('ptc_proc', 'pointcloud', 'ptc_data')
si.RunProcedure('ptc_proc')

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
