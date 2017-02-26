#!/usr/bin/env python

# 1 sphere, 1 bunny and 1 happy buddha in cornel box with pathtracing
# Copyright (c) 2011-2017 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#plugin
si.OpenPlugin('pathtracing_shader', 'PathtracingShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetSampleProperty3('cam1', 'translate', 0, 0.5, 1.85, 0)
si.SetProperty1('cam1', 'fov', 40)

#Texture
si.NewTexture('rock_tex1', '../../jpg/rock.jpg')

#Light
#NOTE Wehn using CG light for direct lighting (not yet)
'''
si.NewLight('light1', 'SphereLight')
si.SetProperty3('light1', 'translate', 0, 1, 0)
si.SetProperty3('light1', 'scale', .2, .05, .2)
si.SetProperty1('light1', 'intensity', 1)
si.SetProperty3('light1', 'color', 1.2, 1.1, 0.9)
si.SetProperty1('light1', 'sample_count', 16)
#si.NewLight('light1', 'GridLight')
#si.SetProperty3('light1', 'translate', 0, 1-.001, 0)
#si.SetProperty3('light1', 'scale', .2, 1, .2)
#si.SetProperty3('light1', 'rotate', 180, 0, 0)
#si.SetProperty1('light1', 'intensity', 1)
#si.SetProperty3('light1', 'color', 1.2, 1.1, 0.9)
#si.SetProperty1('light1', 'sample_count', 16)
'''

#Shader
diff = .8
si.NewShader('floor_shader1', 'pathtracing_shader')
si.SetProperty3('floor_shader1', 'diffuse', diff, diff, diff)

si.NewShader('ceiling_shader1', 'pathtracing_shader')
si.SetProperty3('ceiling_shader1', 'diffuse', diff, diff, diff)

si.NewShader('wall_shader1', 'pathtracing_shader')
si.SetProperty3('wall_shader1', 'diffuse', diff, 0, 0)

si.NewShader('wall_shader2', 'pathtracing_shader')
si.SetProperty3('wall_shader2', 'diffuse', 0, diff, 0)

si.NewShader('wall_shader3', 'pathtracing_shader')
si.SetProperty3('wall_shader3', 'diffuse', diff, diff, diff)

si.NewShader('mirror_shader1', 'pathtracing_shader')
si.SetProperty3('mirror_shader1', 'diffuse', .05, .05, .05)
si.SetProperty3('mirror_shader1', 'reflect', .2, .4, .8)
si.SetProperty1('mirror_shader1', 'ior', 20)

si.NewShader('plastic_shader1', 'pathtracing_shader')
si.SetProperty3('plastic_shader1', 'diffuse', .2, .4, .8)
si.SetProperty3('plastic_shader1', 'reflect', 1, 1, 1)
si.SetProperty3('plastic_shader1', 'specular', .1, .1, .1)

si.NewShader('textured_shader1', 'pathtracing_shader')
si.AssignTexture('textured_shader1', 'diffuse_map', 'rock_tex1')
si.AssignTexture('textured_shader1', 'bump_map', 'rock_tex1')
si.SetProperty1('textured_shader1', 'bump_amplitude', 3)

si.NewShader('glass_shader1', 'pathtracing_shader')
si.SetProperty3('glass_shader1', 'diffuse', 0, 0, 0)
si.SetProperty3('glass_shader1', 'reflect', 1, 1, 1)
si.SetProperty3('glass_shader1', 'refract', 1, 1, 1)
si.SetProperty3('glass_shader1', 'transmit', .2, .1, .0)

si.NewShader('light_shader1', 'pathtracing_shader')
si.SetProperty3('light_shader1', 'diffuse', 0, 0, 0)
emit_r = 1.2
emit_g = 1.1
emit_b = 0.9
emit_x = 20
#NOTE When using CG light for direct lighting (not yet)
'''
emit_x = 10
'''
si.SetProperty3('light_shader1', 'emission', emit_r*emit_x, emit_g*emit_x, emit_b*emit_x)

#Mesh
si.NewMesh('happy_mesh', '../../ply/happy.ply')
si.NewMesh('bunny_mesh', '../../ply/bunny.ply')
si.NewMesh('floor_mesh', '../../ply/floor.ply')
#si.NewMesh('sphere_mesh', '../../ply/sphere.ply')
si.NewMesh('sphere_mesh', '../../ply/sphere_uv.ply')

#ObjectInstance
si.NewObjectInstance('floor1', 'floor_mesh')
si.AssignShader('floor1', 'DEFAULT_SHADING_GROUP', 'floor_shader1')
si.SetProperty3('floor1', 'scale', .1, .1, .1)

si.NewObjectInstance('ceiling1', 'floor_mesh')
si.AssignShader('ceiling1', 'DEFAULT_SHADING_GROUP', 'ceiling_shader1')
si.SetProperty3('ceiling1', 'scale', .1, .1, .1)
si.SetProperty3('ceiling1', 'rotate', 0, 0, 180)
si.SetProperty3('ceiling1', 'translate', 0, 1, 0)

si.NewObjectInstance('wall1', 'floor_mesh')
si.AssignShader('wall1', 'DEFAULT_SHADING_GROUP', 'wall_shader1')
si.SetProperty3('wall1', 'scale', .1, .1, .1)
si.SetProperty3('wall1', 'rotate', 0, 0, -90)
si.SetProperty3('wall1', 'translate', -.5, .5, 0)

si.NewObjectInstance('wall2', 'floor_mesh')
si.AssignShader('wall2', 'DEFAULT_SHADING_GROUP', 'wall_shader2')
si.SetProperty3('wall2', 'scale', .1, .1, .1)
si.SetProperty3('wall2', 'rotate', 0, 0, 90)
si.SetProperty3('wall2', 'translate', .5, .5, 0)

si.NewObjectInstance('wall3', 'floor_mesh')
si.AssignShader('wall3', 'DEFAULT_SHADING_GROUP', 'wall_shader3')
si.SetProperty3('wall3', 'scale', .1, .1, .1)
si.SetProperty3('wall3', 'rotate', 90, 0, 0)
si.SetProperty3('wall3', 'translate', 0, .5, -.5)

si.NewObjectInstance('bunny1', 'bunny_mesh')
si.AssignShader('bunny1', 'DEFAULT_SHADING_GROUP', 'glass_shader1')
si.SetProperty3('bunny1', 'translate', -.23, 0, .21)
si.SetProperty3('bunny1', 'scale', .3, .3, .3)

si.NewObjectInstance('sphere2', 'sphere_mesh')
si.AssignShader('sphere2', 'DEFAULT_SHADING_GROUP', 'textured_shader1')
si.SetProperty3('sphere2', 'translate', .25, .15, .15)
si.SetProperty3('sphere2', 'scale', .15, .15, .15)
si.SetProperty3('sphere2', 'translate', .3, .15, .23)
si.SetProperty3('sphere2', 'scale', .15, .15, .15)
si.SetProperty3('sphere2', 'rotate', -15, 0, 0)

si.NewObjectInstance('happy1', 'happy_mesh')
si.AssignShader('happy1', 'DEFAULT_SHADING_GROUP', 'plastic_shader1')
si.SetProperty3('happy1', 'translate', .0, 0, -.1)
si.SetProperty3('happy1', 'scale', .25, .25, .25)

si.NewObjectInstance('light_source1', 'sphere_mesh')
si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
si.SetProperty3('light_source1', 'translate', 0, 1, 0)
si.SetProperty3('light_source1', 'scale', .2, .05, .2)
#si.NewObjectInstance('light_source1', 'floor_mesh')
#si.AssignShader('light_source1', 'DEFAULT_SHADING_GROUP', 'light_shader1')
#si.SetProperty3('light_source1', 'translate', 0, 1-.001, 0)
#scale = .03
#si.SetProperty3('light_source1', 'scale', scale, scale, scale)

#ObjectGroup
#NOTE Wehn using CG light for direct lighting (not yet)
'''
si.NewObjectGroup('group1')
si.AddObjectToGroup('group1', 'wall1')
si.AddObjectToGroup('group1', 'wall2')
si.AddObjectToGroup('group1', 'wall3')
si.AddObjectToGroup('group1', 'floor1')
si.AddObjectToGroup('group1', 'ceiling1')
si.AddObjectToGroup('group1', 'bunny1')
si.AddObjectToGroup('group1', 'sphere2')
si.AddObjectToGroup('group1', 'happy1')
si.AssignObjectGroup('wall1',    'shadow_target', 'group1')
si.AssignObjectGroup('wall2',    'shadow_target', 'group1')
si.AssignObjectGroup('wall3',    'shadow_target', 'group1')
si.AssignObjectGroup('floor1',   'shadow_target', 'group1')
si.AssignObjectGroup('ceiling1', 'shadow_target', 'group1')
si.AssignObjectGroup('bunny1',   'shadow_target', 'group1')
si.AssignObjectGroup('sphere2',  'shadow_target', 'group1')
si.AssignObjectGroup('happy1',   'shadow_target', 'group1')
'''

#FrameBuffer
si.NewFrameBuffer('fb1', 'rgba')

#Renderer
si.NewRenderer('ren1')
si.AssignCamera('ren1', 'cam1')
si.AssignFrameBuffer('ren1', 'fb1')
si.SetProperty2('ren1', 'resolution', 640, 480)
#si.SetProperty2('ren1', 'resolution', 160, 120)
si.SetProperty2('ren1', 'pixelsamples', 64, 64)
#si.SetProperty2('ren1', 'pixelsamples', 12, 12)

#NOTE CG light for direct lighting (not yet)
'''
si.SetProperty1('ren1', 'max_diffuse_depth', 2)
'''

#TODO Fix adaptive_max_subdivision bug
#si.SetProperty1('ren1', 'sampler_type', 1)
#si.SetProperty1('ren1', 'adaptive_max_subdivision', 1)

#Rendering
si.RenderScene('ren1')

#Output
si.SaveFrameBuffer('fb1', '../pathtracing.fb')

#Run commands
si.Run()
#si.Print()
