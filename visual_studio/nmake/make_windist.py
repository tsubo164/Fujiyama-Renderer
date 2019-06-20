#!/usr/bin/env python
# Copyright (c) 2011-2019 Hiroshi Tsubokawa
# See LICENSE.txt and README.txt

import os, glob, shutil, zipfile, sys

fujiyama_dir = 'Fujiyama-Renderer'

readme_txt = """\
# Copyright (c) 2011-2019 Hiroshi Tsubokawa
# See LICENSE.txt and README.txt

Fujiyama Renderer for Windows 64bit
===================================

Install
    Copy {fujiyama_dir__} to C:\\{fujiyama_dir__}
    set PATH=%PATH%;C\\{fujiyama_dir__}\\bin
    set PYTHONPATH=%PYTHONPATH%;C\\{fujiyama_dir__}\\python

Convert
    ply2mesh.exe teapot.ply  teapot.mesh
    obj2mesh.exe head.obj    head.mesh
    hdr2mip.exe  pisa.hdr    pisa.mip
    jpg2mip.exe  rock.jpg    rock.mip

Render
    scene.exe    teapot.scn
    type teapot.scn | scene.exe
    python       bump_mapping.py
""".format(fujiyama_dir__ = fujiyama_dir)

def mkdir_p(pth):
	if not os.path.exists(pth):
		os.mkdir(pth)

if len(sys.argv) != 2:
    print 'error: provide a version'
    print 'Usage:', __file__, '0.2.7'
    exit()

version = sys.argv[1]

deploy_root = '../../../'
dist_name = 'Fujiyama-Renderer-for-Win-' + version + '-x64'

dist_path    = deploy_root + dist_name
install_path = dist_path     + '/' + fujiyama_dir
bin_path     = install_path + '/bin'
lib_path     = install_path + '/lib'
include_path = install_path + '/include'
scene_path   = install_path + '/scenes'
python_path  = install_path + '/python'

# making folders
mkdir_p(dist_path)
mkdir_p(install_path)
mkdir_p(bin_path)
mkdir_p(lib_path)
mkdir_p(include_path)
mkdir_p(scene_path)
mkdir_p(python_path)

# copying binaries, libraries and scenes
for exe in sorted(glob.glob('./bin/*.exe')):
	print 'copying', exe
	shutil.copy(exe, bin_path)

for binfile in sorted(glob.glob('./bin/*.dll')):
	print 'copying', binfile
	shutil.copy(binfile, bin_path)

for binfile in sorted(glob.glob(deploy_root + 'deploy/bin/*.dll')):
	print 'copying', binfile
	shutil.copy(binfile, bin_path)

for lib in sorted(glob.glob('./bin/*.lib')):
	print 'copying', lib
	shutil.copy(lib, lib_path)

for lib in sorted(glob.glob(deploy_root + 'deploy/lib/*.lib')):
	print 'copying', lib
	shutil.copy(lib, lib_path)

for header in sorted(glob.glob('../../src/*.h')):
	print 'copying', header
	shutil.copy(header, include_path)

for scene in sorted(glob.glob('../../scenes/*.scn')):
	print 'copying', scene
	shutil.copy(scene, scene_path)

for scene in sorted(glob.glob('../../scenes/*.py')):
	print 'copying', scene
	shutil.copy(scene, scene_path)

# copying fujiyama.py
python_api = '../../tools/python_api/fujiyama.py'
print 'copying', python_api
shutil.copy(python_api, python_path)

# making README.txt
readme_name = dist_path + '/README.txt'
readme_file = open(readme_name, 'w')
print 'making', readme_name
readme_file.write(readme_txt)
readme_file.close()

# making LICENSE.txt
license_file_name = dist_path + '/LICENSE.txt'
license_file = open('../../LICENSE', 'r')
license_txt_file = open(license_file_name, 'w')
print 'making', license_file_name
for line in license_file.readlines():
    license_txt_file.write(line)
license_file.close()
license_txt_file.close()

# making zip
z = zipfile.ZipFile(dist_path + '.zip', 'w', zipfile.ZIP_DEFLATED)
for pth, dirs, files in os.walk(dist_path):
    for f in files:
        filename = os.path.join(pth, f)
        archname = os.path.relpath(filename, dist_path)
        archname = os.path.join(dist_name, archname)
        print 'deflating', archname
        z.write(filename, archname)
z.close()
