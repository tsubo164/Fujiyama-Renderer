/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Progress.h"
#include "Triangle.h"
#include "CurveIO.h"
#include "Numeric.h"
#include "MeshIO.h"
#include "Vector.h"
#include "Color.h"
#include "Noise.h"
#include "Array.h"
#include "Mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const char USAGE[] =
"Usage: curvegen [options] inputfile(*.mesh) outputfile(*.crv)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
	struct Progress *progress;

	struct CurveOutput *out;
	struct Mesh *mesh;
	const char *meshfile;
	const char *curvefile;

	struct Vector *P = NULL;
	double *width = NULL;
	struct Color *Cd = NULL;
	int *indices = NULL;
	int *ncurves_on_face = NULL;
	int nfaces = 0;

	struct Vector *sourceP = NULL;
	struct Vector *sourceN = NULL;

	int total_ncurves;
	int total_ncps;

	int curve_id;
	int cp_id;
	int i;

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf("%s", USAGE);
		return 0;
	}

	if (argc != 3) {
		fprintf(stderr, "error: invalid number of arguments.\n");
		fprintf(stderr, "%s", USAGE);
		return -1;
	}
	meshfile = argv[1];
	curvefile = argv[2];

	progress = PrgNew();
	if (progress == NULL) {
		return -1;
	}

	mesh = MshNew();
	if (mesh == NULL) {
		fprintf(stderr, "fatal error: MshNew returned NULL.\n");
		return -1;
	}

	if (MshLoadFile(mesh, meshfile)) {
		const char *err_msg = NULL;
		MshFree(mesh);

		switch (MshGetErrorNo()) {
		case MSH_ERR_NONE:
			err_msg = "";
			break;
		case MSH_ERR_FILE_NOT_EXIST:
			err_msg = "mesh file not found";
			break;
		case MSH_ERR_BAD_MAGIC_NUMBER:
			err_msg = "invalid magic number";
			break;
		case MSH_ERR_BAD_FILE_VERSION:
			err_msg = "invalid file format version";
			break;
		case MSH_ERR_LONG_ATTRIB_NAME:
			err_msg = "too long attribute name was detected";
			break;
		case MSH_ERR_NO_MEMORY:
			err_msg = "no memory to allocate";
			break;
		default:
			err_msg = "";
			break;
		}
		fprintf(stderr, "error: %s: %s\n", err_msg, meshfile);
		return -1;
	}

	nfaces = MshGetFaceCount(mesh);
	printf("nfaces: %d\n", nfaces);

	/* count total_ncurves */
	ncurves_on_face = (int *) malloc(sizeof(int) * nfaces);
	total_ncurves = 0;
	for (i = 0; i < nfaces; i++) {
		struct Vector P0 = {0, 0, 0};
		struct Vector P1 = {0, 0, 0};
		struct Vector P2 = {0, 0, 0};
		double area = 0;

		MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);
		area = TriComputeArea(&P0, &P1, &P2);

		ncurves_on_face[i] = 100000 * area;
		total_ncurves += ncurves_on_face[i];
	}
	printf("total_ncurves: %d\n", total_ncurves);

	total_ncps = 4 * total_ncurves;
	P = VecAlloc(total_ncps);
	width = (double *) malloc(sizeof(double) * total_ncps);
	Cd = ColAlloc(total_ncps);
	indices = (int *) malloc(sizeof(int) * total_ncurves);

	sourceP = VecAlloc(total_ncurves);
	sourceN = VecAlloc(total_ncurves);

	printf("Computing curves' posittion ...\n");
	PrgStart(progress, total_ncurves);

	curve_id = 0;
	for (i = 0; i < nfaces; i++) {
		int j;
		struct Vector P0 = {0, 0, 0};
		struct Vector P1 = {0, 0, 0};
		struct Vector P2 = {0, 0, 0};
		struct Vector N0 = {0, 0, 0};
		struct Vector N1 = {0, 0, 0};
		struct Vector N2 = {0, 0, 0};

		MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);
		MshGetFaceVertexNormal(mesh, i, &N0, &N1, &N2);

		for (j = 0; j < ncurves_on_face[i]; j++) {
			double gravity;
			double u, v, t;
			struct Vector *src_P;
			struct Vector *src_N;

			srand(12.34*i + 1232*j);
			u = (((double) rand()) / RAND_MAX);
			srand(21.43*i + 213*j);
			v = (1-u) * (((double) rand()) / RAND_MAX);

			src_P = &sourceP[curve_id];
			src_N = &sourceN[curve_id];

			t = 1-u-v;
			src_P->x = t * P0.x + u * P1.x + v * P2.x;
			src_P->y = t * P0.y + u * P1.y + v * P2.y;
			src_P->z = t * P0.z + u * P1.z + v * P2.z;

			src_N->x = t * N0.x + u * N1.x + v * N2.x;
			src_N->y = t * N0.y + u * N1.y + v * N2.y;
			src_N->z = t * N0.z + u * N1.z + v * N2.z;

			VEC3_NORMALIZE(src_N);

			srand(i+j);
			gravity = .5 + .5 * (((double) rand()) / RAND_MAX);
			src_N->y -= gravity;
			VEC3_NORMALIZE(src_N);

			curve_id++;

			PrgIncrement(progress);
		}
	}
	assert(curve_id == total_ncurves);
	PrgDone(progress);

	printf("Generating curves ...\n");
	PrgStart(progress, total_ncurves);

	cp_id = 0;
	curve_id = 0;
	for (i = 0; i < total_ncurves; i++) {
		int vtx;

		for (vtx = 0; vtx < 4; vtx++) {
			const double LENGTH = .02;
			struct Vector *dst_P;
			struct Vector *src_P;
			struct Vector *src_N;
			struct Vector noisevec = {0, 0, 0};
			double noiseamp;
			struct Color *dst_Cd;

			srand(12*i + 49*vtx);
			if (vtx > 0) {
				noisevec.x = (((double) rand()) / RAND_MAX);
				noisevec.y = (((double) rand()) / RAND_MAX);
				noisevec.z = (((double) rand()) / RAND_MAX);
			}
			noiseamp = .75 * LENGTH;

			dst_P = &P[cp_id];
			src_P = &sourceP[curve_id];
			src_N = &sourceN[curve_id];

			dst_P->x = src_P->x + noiseamp * noisevec.x + vtx * LENGTH/3. * src_N->x;
			dst_P->y = src_P->y + noiseamp * noisevec.y + vtx * LENGTH/3. * src_N->y;
			dst_P->z = src_P->z + noiseamp * noisevec.z + vtx * LENGTH/3. * src_N->z;

			if (vtx == 0) {
				double *w = &width[cp_id];
				w[0] = .003;
				w[1] = .002;
				w[2] = .001;
				w[3] = .0001;
			}

			dst_Cd = &Cd[cp_id];
			{
				double amp = 1;
				double C_noise = 0;
				struct Color C_dark = {.8, .5, .3};
				struct Color C_light = {.9, .88, .85};
				struct Vector freq = {3, 3, 3};
				struct Vector offset = {0, 1, 0};
				struct Vector src_Q = {0, 0, 0};

				src_Q.x = src_P->x * freq.x + offset.x;
				src_Q.y = src_P->y * freq.y + offset.y;
				src_Q.z = src_P->z * freq.z + offset.z;

				C_noise = amp * PerlinNoise(&src_Q, 2, .5, 2);
				C_noise = SmoothStep(C_noise, .55, .75);
				COL_LERP(dst_Cd, &C_dark, &C_light, C_noise);
			}

			cp_id++;
		}
		indices[curve_id] = 4*i;
		curve_id++;

		PrgIncrement(progress);
	}
	assert(cp_id == total_ncps);
	PrgDone(progress);

	out = CrvOpenOutputFile(curvefile);
	if (out == NULL) {
		MshFree(mesh);
		fprintf(stderr, "error: %s: %s\n", CrvGetErrorMessage(CrvGetErrorNo()), curvefile);
		return -1;
	}

	/* setup CurveOutput */
	out->nverts = total_ncps;
	out->nvert_attrs = 2;
	out->P = P;
	out->width = width;
	out->Cd = Cd;
	out->uv = NULL;
	out->ncurves = total_ncurves;
	out->ncurve_attrs = 1;
	out->indices = indices;

	CrvWriteFile(out);

	/* clean up */
	CrvCloseOutputFile(out);
	MshFree(mesh);
	VecFree(P);
	free(width);
	ColFree(Cd);
	free(indices);
	free(ncurves_on_face);
	VecFree(sourceP);
	VecFree(sourceN);

	return 0;
}

