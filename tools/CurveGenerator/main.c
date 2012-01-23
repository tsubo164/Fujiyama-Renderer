/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "CurveIO.h"
#include "Mesh.h"
#include "MeshIO.h"
#include "Vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char USAGE[] =
"Usage: gencurve [options] inputfile(*.mesh) outputfile(*.crv)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
	struct CurveOutput *out;
	struct Mesh *mesh;
	const char *meshfile;
	const char *curvefile;

	int i;
	int nverts;
	int ncurves;
	int *ncurves_on_face;
	double *cps;
	double *width;
	int *indices;

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf(USAGE);
		return 0;
	}

	if (argc != 3) {
		fprintf(stderr, "error: invalid number of arguments.\n");
		fprintf(stderr, USAGE);
		return -1;
	}
	meshfile = argv[1];
	curvefile = argv[2];

	mesh = MshNew();
	if (mesh == NULL) {
		fprintf(stderr, "fatal error: MshNew returned NULL.\n");
		return -1;
	}

	if (MshLoadFile(mesh, meshfile)) {
		MshFree(mesh);
		fprintf(stderr, "error: %s: %s\n", MshGetErrorMessage(MshGetErrorNo()), meshfile);
		return -1;
	}

	nverts = 4 * mesh->nverts;
	ncurves = mesh->nverts;

	ncurves_on_face = (int *) malloc(sizeof(int) * mesh->nfaces);
	cps = (double *) malloc(sizeof(double) * 3 * nverts);
	width = (double *) malloc(sizeof(double) * nverts);
	indices = (int *) malloc(sizeof(int) * ncurves);

	for (i = 0; i < mesh->nverts; i++) {
		int vtx;
		const double *Pmesh = &mesh->P[i*3];
		const double *Nmesh = &mesh->N[i*3];
		double *cp = &cps[i * 3 * 4];
		double *w = &width[i * 4];

		const double LENGTH = .02;
		for (vtx = 0; vtx < 4; vtx++) {
			cp[vtx * 3 + 0] = Pmesh[0] + vtx * LENGTH/3. * Nmesh[0];
			cp[vtx * 3 + 1] = Pmesh[1] + vtx * LENGTH/3. * Nmesh[1];
			cp[vtx * 3 + 2] = Pmesh[2] + vtx * LENGTH/3. * Nmesh[2];
		}
		w[0] = .003;
		w[1] = .002;
		w[2] = .002;
		w[3] = .0001;
		indices[i] = 4*i;
	}

	out = CrvOpenOutputFile(curvefile);
	if (out == NULL) {
		MshFree(mesh);
		fprintf(stderr, "error: %s: %s\n", CrvGetErrorMessage(CrvGetErrorNo()), curvefile);
		return -1;
	}

	/* setup CurveOutput */
	out->nverts = nverts;
	out->nvert_attrs = 2;
	out->P = cps;
	out->width = width;
	out->uv = NULL;
	out->ncurves = ncurves;
	out->ncurve_attrs = 1;
	out->indices = (int *) indices;

	CrvWriteFile(out);

	/* clean up */
	CrvCloseOutputFile(out);
	MshFree(mesh);
	free(cps);
	free(width);
	free(ncurves_on_face);

	return 0;
}

