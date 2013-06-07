/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#if 0
#include "ObjParser.h"
#include "TexCoord.h"
#include "MeshIO.h"
#include "Array.h"
#include "Mesh.h"
#endif
#include "PointCloudIO.h"
#include "PointCloud.h"
#include "Triangle.h"
#include "Numeric.h"
#include "Memory.h"
#include "MeshIO.h"
#include "Random.h"
#include "Vector.h"
#include "Noise.h"
#include "Mesh.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* TODO TEST */
enum ArgType {
	ARG_NULL = 0,
	ARG_REQUIRED,
	ARG_OPTION
};
struct Option {
	const char type;
	const char short_name;
	const char *long_name;

	struct {
		long Integer;
		double Float;
		const char *String;
	} value;

	const char *desc;
};
#define DEFAULT_INTEGER(val) {(val),    0.,  NULL}
#define DEFAULT_FLOAT(val)   {    0, (val),  NULL}
#define DEFAULT_STRING(val)  {    0,    0., (val)}
#define NULL_OPTION {ARG_NULL, '\0', NULL, DEFAULT_INTEGER(0), NULL}

/*
static struct Option options[] = {
	{ARG_OPTION, 'p', "points-per-area", DEFAULT_FLOAT(1), "# of points per polygon area"},
	NULL_OPTION
};
*/

int OptParse(struct Option *options, int argc, const char **argv);

static const char USAGE[] =
"Usage: mesh2ptc [options] inputfile(*.mesh) outputfile(*.ptc)\n"
"Options:\n"
"  --help         Display this information\n"
"\n";

int main(int argc, const char **argv)
{
	const char *in_filename = NULL;
	const char *out_filename = NULL;
	/*
	int err = 0;
	int n = OptParse(options, argc, argv);

	printf("option count: %d\n", n);
	*/

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf("%s", USAGE);
		return 0;
	}

	if (argc != 3) {
		fprintf(stderr, "error: invalid number of arguments.\n");
		fprintf(stderr, "%s", USAGE);
		return -1;
	}

	in_filename = argv[1];
	out_filename = argv[2];

	{
		struct XorShift xor;
		struct Mesh *mesh = MshNew();
		struct PtcOutputFile *out = PtcOpenOutputFile(out_filename);
		struct Vector *P = NULL;
		double *radius = NULL;
		int *point_count_list = 0;
		int total_point_count = 0;
		int face_count = 0;
		int point_id = 0;
		int i;

		MshLoadFile(mesh, in_filename);

		face_count = MshGetFaceCount(mesh);
		point_count_list = MEM_ALLOC_ARRAY(int, face_count);

		for (i = 0; i < face_count; i++) {
			struct Vector P0 = {0, 0, 0};
			struct Vector P1 = {0, 0, 0};
			struct Vector P2 = {0, 0, 0};
			struct Vector center = {0, 0, 0};
			double noise_val = 0;
			double area = 0;
			int npt_on_face = 0;

			MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);

			center.x = (P0.x + P1.x + P2.x) / 3;
			center.y = (P0.y + P1.y + P2.y) / 3;
			center.z = (P0.z + P1.z + P2.z) / 3;
			center.x *= 2.5;
			center.y *= 2.5;
			center.z *= 2.5;
			noise_val = PerlinNoise(&center, 2, .5, 8);
			noise_val = Fit(noise_val, -.2, 1, 0, 1);
			/*
			noise_val = MAX(noise_val, 0);
			*/

			area = TriComputeArea(&P0, &P1, &P2);
			area *= noise_val;
			npt_on_face = (int) 2000000 * area;

			total_point_count += npt_on_face;
			point_count_list[i] = npt_on_face;
		}
		printf("total_point_count %d\n", total_point_count);

		P = MEM_ALLOC_ARRAY(struct Vector, total_point_count);
		radius = MEM_ALLOC_ARRAY(double, total_point_count);

		XorInit(&xor);
		point_id = 0;
		for (i = 0; i < face_count; i++) {
			struct Vector P0 = {0, 0, 0};
			struct Vector P1 = {0, 0, 0};
			struct Vector P2 = {0, 0, 0};
			struct Vector N0 = {0, 0, 0};
			struct Vector N1 = {0, 0, 0};
			struct Vector N2 = {0, 0, 0};
			const int npt_on_face = point_count_list[i];
			int j;

			MshGetFaceVertexPosition(mesh, i, &P0, &P1, &P2);
			MshGetFaceVertexNormal(mesh, i, &N0, &N1, &N2);

			for (j = 0; j < npt_on_face; j++) {
				struct Vector normal = {0, 0, 0};
				/*
				struct Vector noise_val = {0, 0, 0};
				struct Vector noise_pos = {0, 0, 0};
				*/
				struct Vector *P_out = &P[point_id];
				double u = 0;
				double v = 0;
				double t = 0;
				double n = 0;

				u = XorNextFloat01(&xor);
				v = (1 - u) * XorNextFloat01(&xor);

				TriComputeNormal(&normal, &N0, &N1, &N2, u, v);

				t = 1 - u - v;
				P_out->x = t * P0.x + u * P1.x + v * P2.x;
				P_out->y = t * P0.y + u * P1.y + v * P2.y;
				P_out->z = t * P0.z + u * P1.z + v * P2.z;

				n = XorNextFloat01(&xor);
				n *= -.05;
				P_out->x += n * normal.x;
				P_out->y += n * normal.y;
				P_out->z += n * normal.z;

				radius[point_id] = .01 * .2;

#if 0
				noise_pos.x = 1.5 * P_out->x;
				noise_pos.y = 1.5 * P_out->y;
				noise_pos.z = 1.5 * P_out->z;

				PerlinNoise3d(&noise_pos, 2, .5, 8, &noise_val);

				P_out->x += .3 * noise_val.x;
				P_out->y += .3 * noise_val.y;
				P_out->z += .3 * noise_val.z;
#endif

				point_id++;
			}
		}

		PtcSetOutputPosition(out, P, total_point_count);
		PtcSetOutputAttributeDouble(out, "radius", radius);

		PtcWriteFile(out);

		PtcCloseOutputFile(out);
		MshFree(mesh);

		MEM_FREE(point_count_list);
		MEM_FREE(P);
		MEM_FREE(radius);
	}

	return 0;
}

int OptParse(struct Option *options, int argc, const char **argv)
{
	int required_count = 0;
	int option_count = 0;
	int i;

	for (;;) {
		struct Option *opt = &options[option_count];

		if (opt->type == ARG_REQUIRED) {
			required_count++;
		}
		else if (opt->type == ARG_OPTION) {
			option_count++;
		}
		else if (opt->type == ARG_NULL) {
			break;
		}
	}

	for (i = 0; i < argc; i++) {
		const char *arg = argv[i];

		if (arg[0] == '-' && isalpha(arg[1]) && arg[2] == '\0') {
			printf("short name: [%s]\n", arg);
		}
		else if (arg[0] == '-' && arg[1] == '-') {
			printf("long name: [%s]\n", arg);
		}
		else {
			printf("unrecognized option: [%s]\n", arg);
		}
	}

	return option_count;
}

