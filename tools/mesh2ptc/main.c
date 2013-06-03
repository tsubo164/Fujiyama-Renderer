/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#if 0
#include "ObjParser.h"
#include "TexCoord.h"
#include "Triangle.h"
#include "MeshIO.h"
#include "Array.h"
#include "Mesh.h"
#endif
#include "PointCloudIO.h"
#include "PointCloud.h"
#include "Memory.h"
#include "MeshIO.h"
#include "Vector.h"
#include "Mesh.h"

#include <stdio.h>
#include <string.h>

/* TODO TEST */
enum ArgType {
	ARG_REQUIRED = 0,
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

static struct Option options[] = {
	{ARG_OPTION, 'f', "filename", DEFAULT_STRING(NULL), "input file name"},
	{ARG_OPTION, 'a', "ascci",    DEFAULT_INTEGER(0),   "write ascci data"}
};

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
		struct Mesh *mesh = MshNew();
		struct PtcOutputFile *out = PtcOpenOutputFile(out_filename);
		struct Vector *P = NULL;
		double *radius = NULL;
		int point_count = 0;
		int i;

		MshLoadFile(mesh, in_filename);

		point_count = MshGetVertexCount(mesh);

		P = MEM_ALLOC_ARRAY(struct Vector, point_count);
		radius = MEM_ALLOC_ARRAY(double, point_count);

		for (i = 0; i < point_count; i++) {
			MshGetVertexPosition(mesh, i, &P[i]);
			radius[i] = .05;
		}

		PtcSetOutputPosition(out, P, point_count);
		PtcSetOutputAttributeDouble(out, "radius", radius);

		PtcWriteFile(out);

		PtcCloseOutputFile(out);
		MshFree(mesh);
	}
	{
		/*
		struct PointCloud *ptc = PtcNew();
		int i;

		PtcLoadFile(ptc, out_filename);

		PtcFree(ptc);
		*/
	}
#if 0
	{
		struct PtcOutputFile *out = PtcOpenOutputFile(out_filename);
		const struct Vector P[] = {
			/*
			{0, 0, 0}
			*/
			{1, 2, 3},
			{3, 1, 2},
			{2, 3, 1}
		};
		const double radius[] = {
			.5,
			.2,
			.4,
		};
		const int point_count = sizeof(P)/sizeof(P[0]);

		PtcSetOutputPosition(out, P, point_count);
		PtcSetOutputAttributeDouble(out, "radius", radius);

		PtcWriteFile(out);

		PtcCloseOutputFile(out);
	}
	{
		struct PtcInputFile *in = PtcOpenInputFile(out_filename);
		struct Vector P[] = {
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0}
		};
		int i;
		double radius[] = {
			0.,
			0.,
			0.
		};
		double hoge[] = {
			1.,
			2.,
			3.
		};
		int point_count = 0;

		PtcReadHeader(in);
		point_count = PtcGetInputPointCount(in);

		printf("#  point_count: %d\n", point_count);

		PtcSetInputPosition(in, P);
		PtcSetInputAttributeDouble(in, "radius", radius);
		PtcSetInputAttributeDouble(in, "hoge", hoge);
		PtcReadData(in);

		for (i = 0; i < point_count; i++) {
			printf("#    P: ");
			VecPrint(&P[i]);
		}
		for (i = 0; i < point_count; i++) {
			printf("#    radius: %g\n", radius[i]);
		}
		for (i = 0; i < point_count; i++) {
			printf("#    hoge: %g\n", hoge[i]);
		}

		PtcCloseInputFile(in);
	}
#endif

	return 0;
}

