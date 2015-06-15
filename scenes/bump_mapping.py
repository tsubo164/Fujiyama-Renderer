#!/usr/bin/env python

# 4 textured spheres with 1 dome light with an HDRI
# Copyright (c) 2011-2014 Hiroshi Tsubokawa

import fujiyama

si = fujiyama.SceneInterface()

#Plugins
si.OpenPlugin('ConstantShader')
si.OpenPlugin('PlasticShader')

#Camera
si.NewCamera('cam1', 'PerspectiveCamera')
si.SetProperty3('cam1', 'translate', 0, 0, 10)

# Rotation of dome light and dome object used for background iamge.
rot = 110

#Texture
si.NewTexture('env_tex1', '../../hdr/pisa.hdr')
si.NewTexture('rock_tex1', '../../jpg/rock.jpg')
si.NewTexture('rust_tex1', '../../jpg/rust.jpg')
si.NewTexture('concrete_tex1', '../../jpg/concrete.jpg')
si.NewTexture('pattern_tex1', '../../jpg/pattern.jpg')

#Light
si.NewLight('light1', 'DomeLight')
si.SetProperty3('light1', 'rotate', 0, rot, 0)
# You chang the number of samples on the env map. the default is 16.
#si.SetProperty1('light1', 'sample_count', 256)
si.SetProperty1('light1', 'sample_count', 32)
si.AssignTexture('light1', 'environment_map', 'env_tex1');

#Shader
si.NewShader('sphere_shader1', 'PlasticShader')
si.AssignTexture('sphere_shader1', 'diffuse_map', 'rock_tex1')
si.AssignTexture('sphere_shader1', 'bump_map', 'rock_tex1')
si.SetProperty1('sphere_shader1', 'bump_amplitude', 1)

si.NewShader('sphere_shader2', 'PlasticShader')
si.AssignTexture('sphere_shader2', 'diffuse_map', 'rust_tex1')
si.AssignTexture('sphere_shader2', 'bump_map', 'rust_tex1')
si.SetProperty1('sphere_shader2', 'bump_amplitude', 1)

si.NewShader('sphere_shader3', 'PlasticShader')
si.AssignTexture('sphere_shader3', 'diffuse_map', 'concrete_tex1')
si.AssignTexture('sphere_shader3', 'bump_map', 'concrete_tex1')
si.SetProperty1('sphere_shader3', 'bump_amplitude', 1)

si.NewShader('sphere_shader4', 'PlasticShader')
si.AssignTexture('sphere_shader4', 'diffuse_map', 'pattern_tex1')
si.AssignTexture('sphere_shader4', 'bump_map', 'pattern_tex1')
si.SetProperty1('sphere_shader4', 'bump_amplitude', -1)

si.SetProperty3('sphere_shader4', 'diffuse', .5, .5, .5)
si.SetProperty1('sphere_shader4', 'ior', 10)

si.NewShader('dome_shader', 'ConstantShader')
si.AssignTexture('dome_shader', 'texture', 'env_tex1')

#Mesh
si.NewMesh('sphere_mesh', '../../ply/sphere_uv.ply')
si.NewMesh('dome_mesh', '../../ply/dome.ply')

#ObjectInstance
si.NewObjectInstance('sphere1', 'sphere_mesh')
si.SetProperty3('sphere1', 'translate', -1.7, 1.2, 0)
si.AssignShader('sphere1', 'DEFAULT_SHADING_GROUP', 'sphere_shader1')

si.NewObjectInstance('sphere2', 'sphere_mesh')
si.SetProperty3('sphere2', 'translate', 1.7, 1.2, 0)
si.SetProperty3('sphere2', 'rotate', 0, 20, 0)
si.AssignShader('sphere2', 'DEFAULT_SHADING_GROUP', 'sphere_shader2')

si.NewObjectInstance('sphere3', 'sphere_mesh')
si.SetProperty3('sphere3', 'translate', -1.7, -1.2, 0)
si.SetProperty3('sphere3', 'rotate', 0, 20, 0)
si.AssignShader('sphere3', 'DEFAULT_SHADING_GROUP', 'sphere_shader3')

si.NewObjectInstance('sphere4', 'sphere_mesh')
si.SetProperty3('sphere4', 'translate', 1.7, -1.2, 0)
si.AssignShader('sphere4', 'DEFAULT_SHADING_GROUP', 'sphere_shader4')

si.NewObjectInstance('dome1', 'dome_mesh')
si.SetProperty3('dome1', 'rotate', 0, rot, 0)
si.SetProperty3('dome1', 'scale', -.5, .5, .5)
si.AssignShader('dome1', 'DEFAULT_SHADING_GROUP', 'dome_shader')

#ObjectGroup
# Create shadow_target for sphere1.
# Since 'DomeLight' has infinite distance, we need to exclude
# 'dome1' object which is for just background image.
si.NewObjectGroup('shadow_target1')
si.AddObjectToGroup('shadow_target1', 'sphere1')
si.AssignObjectGroup('sphere1', 'shadow_target', 'shadow_target1')
# and each sphere reacts to only 'dome1' and itself, not to the other spheres
si.NewObjectGroup('reflect_target1')
si.AddObjectToGroup('reflect_target1', 'dome1')
si.AddObjectToGroup('reflect_target1', 'sphere1')
si.AssignObjectGroup('sphere1', 'reflect_target', 'reflect_target1')

si.NewObjectGroup('shadow_target2')
si.AddObjectToGroup('shadow_target2', 'sphere2')
si.AssignObjectGroup('sphere2', 'shadow_target', 'shadow_target2')
si.NewObjectGroup('reflect_target2')
si.AddObjectToGroup('reflect_target2', 'dome1')
si.AddObjectToGroup('reflect_target2', 'sphere2')
si.AssignObjectGroup('sphere2', 'reflect_target', 'reflect_target2')

si.NewObjectGroup('shadow_target3')
si.AddObjectToGroup('shadow_target3', 'sphere3')
si.AssignObjectGroup('sphere3', 'shadow_target', 'shadow_target3')
si.NewObjectGroup('reflect_target3')
si.AddObjectToGroup('reflect_target3', 'dome1')
si.AddObjectToGroup('reflect_target3', 'sphere3')
si.AssignObjectGroup('sphere3', 'reflect_target', 'reflect_target3')

si.NewObjectGroup('shadow_target4')
si.AddObjectToGroup('shadow_target4', 'sphere4')
si.AssignObjectGroup('sphere4', 'shadow_target', 'shadow_target4')
si.NewObjectGroup('reflect_target4')
si.AddObjectToGroup('reflect_target4', 'dome1')
si.AddObjectToGroup('reflect_target4', 'sphere4')
si.AssignObjectGroup('sphere4', 'reflect_target', 'reflect_target4')

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
si.SaveFrameBuffer('fb1', '../bump_mapping.fb')

#Run commands
si.Run()
#si.Print()
