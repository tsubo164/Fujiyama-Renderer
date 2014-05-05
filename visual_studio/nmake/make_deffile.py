#!/usr/bin/env python
#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE and README

import glob

src_dir = '../../src'
def_file = 'def/libscene.def'

lines = [
'LIBRARY libscene\n',
'EXPORTS\n']

headers = sorted(glob.glob(src_dir + '/*.h'))
for header in headers:
	with open(header, 'r') as infile:
		for l in infile.readlines():
			if l.startswith('extern ') and l.find('"C"') == -1:
				l = l.replace('extern', '')
				l = l.replace('struct', '')
				l = l.replace('const', '')
				l = l.replace('*', '')
				l = l.strip()
				l = l.split('(')[0]
				l = l.split(' ')[1]
				lines.append('\t' + l + '\n')
	infile.closed

outfile = open(def_file, 'w')
outfile.writelines(lines)
outfile.close()
