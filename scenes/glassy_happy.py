#!/usr/bin/env python

# glassy armadillo, happy buddha and horse
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugins
si.OpenPlugin('constant_shader', 'ConstantShader')
si.OpenPlugin('plastic_shader', 'PlasticShader')
si.OpenPlugin('glass_shader', 'GlassShader')
si.OpenPlugin('stanfordply_procedure', 'StanfordPlyProcedure')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 1.5, 7)

#Light
si.NewLight('light1', 'PointLight')
si.SetProperty3('light1', 'translate', 5, 12, -5)

#Shader
si.NewShader('armadillo_shader', 'glass_shader')
si.SetProperty3('armadillo_shader', 'filter_color', .75, .025, .4)

si.NewShader('happy_shader', 'glass_shader')
si.SetProperty3('happy_shader', 'filter_color', .4, .75, .025)

si.NewShader('horse_shader', 'glass_shader')
si.SetProperty3('horse_shader', 'filter_color', .025, .4, .75)

si.NewShader('dome_shader', 'constant_shader')
si.NewShader('floor_shader', 'plastic_shader')

#Mesh
si.NewMesh('armadillo_mesh', 'null')
si.NewMesh('happy_mesh', 'null')
si.NewMesh('horse_mesh', 'null')
si.NewMesh('dome_mesh', 'null')
si.NewMesh('floor_mesh', 'null')

#Procedure
si.NewProcedure('armadillo_proc', 'stanfordply_procedure')
si.AssignMesh('armadillo_proc', 'mesh', 'armadillo_mesh')
si.SetStringProperty('armadillo_proc', 'filepath', '../../ply/armadillo.ply')
si.SetStringProperty('armadillo_proc', 'io_mode', 'r')
si.RunProcedure('armadillo_proc')

si.NewProcedure('happy_proc', 'stanfordply_procedure')
si.AssignMesh('happy_proc', 'mesh', 'happy_mesh')
si.SetStringProperty('happy_proc', 'filepath', '../../ply/happy.ply')
si.SetStringProperty('happy_proc', 'io_mode', 'r')
si.RunProcedure('happy_proc')

si.NewProcedure('horse_proc', 'stanfordply_procedure')
si.AssignMesh('horse_proc', 'mesh', 'horse_mesh')
si.SetStringProperty('horse_proc', 'filepath', '../../ply/horse.ply')
si.SetStringProperty('horse_proc', 'io_mode', 'r')
si.RunProcedure('horse_proc')

si.NewProcedure('dome_proc', 'stanfordply_procedure')
si.AssignMesh('dome_proc', 'mesh', 'dome_mesh')
si.SetStringProperty('dome_proc', 'filepath', '../../ply/dome.ply')
si.SetStringProperty('dome_proc', 'io_mode', 'r')
si.RunProcedure('dome_proc')

si.NewProcedure('floor_proc', 'stanfordply_procedure')
si.AssignMesh('floor_proc', 'mesh', 'floor_mesh')
si.SetStringProperty('floor_proc', 'filepath', '../../ply/floor.ply')
si.SetStringProperty('floor_proc', 'io_mode', 'r')
si.RunProcedure('floor_proc')

#ObjectInstance
si.NewObjectInstance('armadillo1', 'armadillo_mesh')
si.AssignShader('armadillo1', 'DEFAULT_SHADING_GROUP', 'armadillo_shader')
si.SetProperty3('armadillo1', 'translate', -2, 0, -3)

si.NewObjectInstance('happy1', 'happy_mesh')
si.AssignShader('happy1', 'DEFAULT_SHADING_GROUP', 'happy_shader')

si.NewObjectInstance('horse1', 'horse_mesh')
si.AssignShader('horse1', 'DEFAULT_SHADING_GROUP', 'horse_shader')
si.SetProperty3('horse1', 'translate', 2, 0, -3)

si.NewObjectInstance('dome1', 'dome_mesh')
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader')

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty1('ren1', 'max_refract_depth', 10)
si.SetProperty1('ren1', 'cast_shadow', 0)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../glassy_happy.fb')

#Run commands
si.Run()
#si.Print()

