#!/usr/bin/env python
#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE.txt and README.txt

import os, glob, shutil, zipfile, sys

readme_txt = """\
#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE.txt and README.txt

Fujiyama Renderer for Windows 64bit
===================================

Install
    Copy Fujiyama to C:\\Fujiyama
    set PATH=%PATH%;C\\Fujiyama\\bin
    set PYTHONPATH=%PYTHONPATH%;C\\Fujiyama\\python

Convert
    ply2mesh.exe teapot.ply  teapot.mesh
    obj2mesh.exe head.obj    head.mesh
    hdr2mip.exe  pisa.hdr    pisa.mip
    jpg2mip.exe  rock.jpg    rock.mip

Render
    scene.exe    teapot.scn
    type teapot.scn | scene.exe
    python       bump_mapping.py
"""

def mkdir_p(pth):
	if not os.path.exists(pth):
		os.mkdir(pth)

if len(sys.argv) != 2:
    print 'error: provide a version'
    print 'Usage:', __file__, 'v0.2.6'
    exit()

version = sys.argv[1]

deploy_root = '../../../'

top_dir = deploy_root + 'Fujiyama-Renderer-for-Win-' + version + '-x64'
install_dir = top_dir     + '/Fujiyama'
bin_dir     = install_dir + '/bin'
lib_dir     = install_dir + '/lib'
include_dir = install_dir + '/include'
scene_dir   = install_dir + '/scenes'
python_dir  = install_dir + '/python'

# making folders
mkdir_p(top_dir)
mkdir_p(install_dir)
mkdir_p(bin_dir)
mkdir_p(lib_dir)
mkdir_p(include_dir)
mkdir_p(scene_dir)
mkdir_p(python_dir)

# copying binaries, libraries and scenes
for exe in sorted(glob.glob('./bin/*.exe')):
	print 'copying', exe
	shutil.copy(exe, bin_dir)

for binfile in sorted(glob.glob('./bin/*.dll')):
	print 'copying', binfile
	shutil.copy(binfile, bin_dir)

for binfile in sorted(glob.glob(deploy_root + 'deploy/bin/*.dll')):
	print 'copying', binfile
	shutil.copy(binfile, bin_dir)

for lib in sorted(glob.glob('./bin/*.lib')):
	print 'copying', lib
	shutil.copy(lib, lib_dir)

for lib in sorted(glob.glob(deploy_root + 'deploy/lib/*.lib')):
	print 'copying', lib
	shutil.copy(lib, lib_dir)

for header in sorted(glob.glob('../../src/*.h')):
	print 'copying', header
	shutil.copy(header, include_dir)

for scene in sorted(glob.glob('../../scenes/*.scn')):
	print 'copying', scene
	shutil.copy(scene, scene_dir)

for scene in sorted(glob.glob('../../scenes/*.py')):
	print 'copying', scene
	shutil.copy(scene, scene_dir)

# copying fujiyama.py
python_api = '../../tools/python_api/fujiyama.py'
print 'copying', python_api
shutil.copy(python_api, python_dir)

# making README.txt
readme_name = top_dir + '/README.txt'
readme_file = open(readme_name, 'w')
print 'making', readme_name
readme_file.write(readme_txt)
readme_file.close()

# making LICENSE.txt
license_file_name = top_dir + '/LICENSE.txt'
license_file = open('../../LICENSE', 'r')
license_txt_file = open(license_file_name, 'w')
print 'making', license_file_name
for line in license_file.readlines():
    license_txt_file.write(line)
license_file.close()
license_txt_file.close()

# making zip
z = zipfile.ZipFile(top_dir + '.zip', 'w', zipfile.ZIP_DEFLATED)
for pth, dirs, files in os.walk(top_dir):
    for f in files:
        filename = os.path.join(pth, f)
        archname = os.path.relpath(filename, top_dir)
        print 'deflating', archname
        z.write(filename, archname)
z.close()
