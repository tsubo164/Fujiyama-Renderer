// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef PLY2MESH_H
#define PLY2MESH_H

namespace fj {
class Mesh;
}

int ReadPlyFile(const char *filename, fj::Mesh &mesh);

#endif // XXX_H
