/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "CurveIO.h"
#include "MeshIO.h"
#include "Mesh.h"
#include "Vector.h"
#include "Array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

	struct Double3Array cps;
	struct Double1Array width;
	struct Int1Array indices;
	struct Int1Array ncurves_on_face;

	struct Double3Array curveP;
	struct Double3Array curveN;

	int total_curves_on_face;
	int total_ncurves;
	int total_ncps;

	int curve_id;
	int cp_id;
	int i;

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

	/* count total_ncurves */
	ncurves_on_face = Int1ArrayNew(mesh->nfaces);
	total_curves_on_face = 0;
	for (i = 0; i < mesh->nfaces; i++) {
		const double *v0, *v1, *v2;
		double a[3];
		double b[3];
		double cross[3];
		double area;
		int *ncurves;

		MshGetFaceVertex(mesh, i, &v0, &v1, &v2);

		VEC3_SUB(a, v1, v0);
		VEC3_SUB(b, v2, v0);
		VEC3_CROSS(cross, a, b);
		area = .5 * VEC3_LEN(cross);

		ncurves = Int1ArrayGetWritable(ncurves_on_face, i);
		*ncurves = 100000 * area;
		total_curves_on_face += *ncurves;
	}
	total_ncurves = total_curves_on_face;
	printf("total_curves_on_face: %d\n", total_curves_on_face);
	printf("total_ncurves: %d\n", total_ncurves);

	total_ncps = 4 * total_ncurves;
	cps = Double3ArrayNew(total_ncps);
	width = Double1ArrayNew(total_ncps);
	indices = Int1ArrayNew(total_ncurves);

	curveP = Double3ArrayNew(total_ncurves);
	curveN = Double3ArrayNew(total_ncurves);

	curve_id = 0;
	for (i = 0; i < mesh->nfaces; i++) {
		int j;
		int *ncurves;
		int i0, i1, i2;
		const double *P0, *P1, *P2;
		const double *N0, *N1, *N2;

		i0 = mesh->indices[3*i+0];
		i1 = mesh->indices[3*i+1];
		i2 = mesh->indices[3*i+2];

		N0 = &mesh->N[3*i0];
		N1 = &mesh->N[3*i1];
		N2 = &mesh->N[3*i2];

		P0 = &mesh->P[3*i0];
		P1 = &mesh->P[3*i1];
		P2 = &mesh->P[3*i2];

		ncurves = Int1ArrayGetWritable(ncurves_on_face, i);
		for (j = 0; j < *ncurves; j++) {
			double gravity;
			double u, v, t;
			double *p;
			double *n;

			srand(12.34*i + 1232*j);
			u = (((double) rand()) / RAND_MAX);
			srand(21.43*i + 213*j);
			v = (1-u) * (((double) rand()) / RAND_MAX);

			p = Double3ArrayGetWritable(curveP, curve_id);
			n = Double3ArrayGetWritable(curveN, curve_id);

			t = 1-u-v;
			p[0] = t * P0[0] + u * P1[0] + v * P2[0];
			p[1] = t * P0[1] + u * P1[1] + v * P2[1];
			p[2] = t * P0[2] + u * P1[2] + v * P2[2];

			n[0] = t * N0[0] + u * N1[0] + v * N2[0];
			n[1] = t * N0[1] + u * N1[1] + v * N2[1];
			n[2] = t * N0[2] + u * N1[2] + v * N2[2];

			VEC3_NORMALIZE(n);

			srand(i+j);
			gravity = .5 + .5 * (((double) rand()) / RAND_MAX);
			n[1] -= gravity;
			VEC3_NORMALIZE(n);

			curve_id++;
		}
	}
	assert(curve_id == total_ncurves);

	cp_id = 0;
	curve_id = 0;
	for (i = 0; i < total_ncurves; i++) {
		int vtx;
		int *idx;

		for (vtx = 0; vtx < 4; vtx++) {
			const double LENGTH = .02;
			double *cp;
			double *p;
			double *n;
			double noisevec[3] = {0};
			double noiseamp;

			srand(12*i + 49*vtx);
			if (vtx > 0) {
				noisevec[0] = (((double) rand()) / RAND_MAX);
				noisevec[1] = (((double) rand()) / RAND_MAX);
				noisevec[2] = (((double) rand()) / RAND_MAX);
			}
			noiseamp = .75 * LENGTH;

			cp = Double3ArrayGetWritable(cps, cp_id);
			p = Double3ArrayGetWritable(curveP, curve_id);
			n = Double3ArrayGetWritable(curveN, curve_id);

			cp[0] = p[0] + noiseamp * noisevec[0] + vtx * LENGTH/3. * n[0];
			cp[1] = p[1] + noiseamp * noisevec[1] + vtx * LENGTH/3. * n[1];
			cp[2] = p[2] + noiseamp * noisevec[2] + vtx * LENGTH/3. * n[2];

			if (vtx == 0) {
				double *w = Double1ArrayGetWritable(width, cp_id);
				w[0] = .003;
				w[1] = .002;
				w[2] = .002;
				w[3] = .0001;
			}

			cp_id++;
		}

		idx = Int1ArrayGetWritable(indices, i);
		*idx = 4*i;

		curve_id++;
	}
	assert(cp_id == total_ncps);

	out = CrvOpenOutputFile(curvefile);
	if (out == NULL) {
		MshFree(mesh);
		fprintf(stderr, "error: %s: %s\n", CrvGetErrorMessage(CrvGetErrorNo()), curvefile);
		return -1;
	}

	/* setup CurveOutput */
	out->nverts = total_ncps;
	out->nvert_attrs = 2;
	out->P = Double3ArrayGetWritable(cps, 0);
	out->width = Double1ArrayGetWritable(width, 0);
	out->uv = NULL;
	out->ncurves = total_ncurves;
	out->ncurve_attrs = 1;
	out->indices = Int1ArrayGetWritable(indices, 0);

	CrvWriteFile(out);

	/* clean up */
	CrvCloseOutputFile(out);
	MshFree(mesh);
	Double3ArrayFree(cps);
	Double1ArrayFree(width);
	Int1ArrayFree(indices);
	Int1ArrayFree(ncurves_on_face);
	Double3ArrayFree(curveP);
	Double3ArrayFree(curveN);

	return 0;
}

