#!/usr/bin/env python

# 1 surface wisps with 1 point light
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('volume_shader', 'VolumeShader')
si.OpenPlugin('SurfaceWispsProcedure', 'SurfaceWispsProcedure')
si.OpenPlugin('stanfordply_procedure', 'StanfordPlyProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 2, 6)
si.SetProperty3('cam1', 'rotate', -11.309932474020213, 0, 0)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', 10, 12, 10)

#Shader
si.NewShader('floor_shader', 'plastic_shader')
si.SetProperty3('floor_shader', 'diffuse', .2, .25, .3)
si.SetProperty1('floor_shader', 'ior', 2)

si.NewShader('volume_shader1', 'volume_shader')
si.SetProperty3('volume_shader1', 'diffuse', .2, .2, .2)

si.NewShader('dome_shader', 'constant_shader')
si.SetProperty3('dome_shader', 'diffuse', .8, .8, .8)

#Turbulence
si.NewTurbulence('turbulence_data')
si.SetProperty3('turbulence_data', 'frequency', 1.5, 1.5, .5)
si.SetProperty3('turbulence_data', 'amplitude', 1, 1, 1)
si.SetProperty3('turbulence_data', 'offset', .6, 0, 0)

#Volume
si.NewVolume('volume_data')
si.SetProperty3('volume_data', 'bounds_min', -1, -1, -1)
si.SetProperty3('volume_data', 'bounds_max', 1, 1, 1)
#si.SetProperty3('volume_data', 'resolution', 50, 50, 50)
si.SetProperty3('volume_data', 'resolution', 500, 500, 500)

#Mesh
si.NewMesh('floor_mesh',  'null')
si.NewMesh('dome_mesh',   'null')

#Procedure
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

si.NewProcedure('surfacewisps_proc', 'SurfaceWispsProcedure')
si.AssignVolume('surfacewisps_proc', 'volume', 'volume_data')
si.AssignTurbulence('surfacewisps_proc', 'turbulence', 'turbulence_data')
si.RunProcedure('surfacewisps_proc')

#ObjectInstance
si.NewObjectInstance('volume1', 'volume_data')
si.AssignShader('volume1', 'DEFAULT_SHADING_GROUP', 'volume_shader1')
si.SetProperty3('volume1', 'translate', 0, .75, 0)

si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')

si.NewObjectInstance('dome1', 'dome_mesh')
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

#ObjectGroup
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'volume1')
si.AssignObjectGroup('volume1', 'shadow_target', 'group1')
si.AssignObjectGroup('floor1', 'shadow_target', 'group1')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Properties
#si.ShowPropertyList('SurfaceWispsProcedure')
#si.ShowPropertyList('volume_shader')
#si.ShowPropertyList('Renderer')
#si.ShowPropertyList('Volume')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)

si.SetProperty1('ren1', 'raymarch_step', .01)
si.SetProperty1('ren1', 'raymarch_shadow_step', .02)
si.SetProperty1('ren1', 'raymarch_reflect_step', .02)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../surface_wisps.fb')

#Run commands
si.Run()
#si.Print()

