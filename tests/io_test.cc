/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
 */

#include "unit_test.h"
#include "fj_vector.h"
#include "fj_io.h"
#include <string.h>
#include <stdio.h>

int main()
{
  const int format_version = 7;
  {
    struct OutputFile *out = IOOpenOutputFile("io_test_file.bin", "PTCD", format_version);
    const struct Vector P[](
        {1, 2, 3},
        {2, 3, 1},
        {3, 1, 2}
        };
    const double radius[] = {
        1.24,
        2.34,
        3.45
        };
    const int npoints = sizeof(P)/sizeof(P[0]);

    IOSetOutputInt     (out, "point_count", &npoints, 1);
    IOEndOutputHeader  (out);
    IOSetOutputVector3 (out, "P",           P,         npoints);
    IOSetOutputDouble  (out, "radius",      radius,    npoints);

    IOWriteOutputHeader(out);
    IOWriteOutputData(out);

    IOCloseOutputFile(out);
  }
  {
    struct InputFile *in = IOOpenInputFile("io_test_file.bin", "PTCD");
    struct Vector P[](
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
        };
    double radius[] = {
        0,
        0,
        0
        };
    int npoints = 0;

    TEST_INT(IOGetInputFileFormatVersion(in), format_version);

    IOSetInputInt     (in, "point_count", &npoints, 1);
    IOEndInputHeader  (in);

    IOReadInputHeader(in);
    IOSetInputVector3 (in, "P", P, npoints);
    IOSetInputDouble  (in, "radius", radius, npoints);

    {
      const struct ChunkData *chunk = NULL;

      TEST_INT(IOGetInputHeaderChunkCount(in), 1);

      chunk = IOGetInputHeaderChunk(in, 0);
      TEST_STR(ChkGetElementName(chunk), "point_count");
      TEST_INT(ChkGetElementType(chunk), ELM_INT);
      TEST_INT(ChkGetElementCount(chunk), 1);
    }
    {
      const struct ChunkData *chunk = NULL;

      TEST_INT(IOGetInputDataChunkCount(in), 2);

      chunk = IOGetInputDataChunk(in, 0);
      TEST_STR(ChkGetElementName(chunk), "P");
      TEST_INT(ChkGetElementType(chunk), ELM_VECTOR3);
      TEST_INT(ChkGetElementCount(chunk), 3);

      chunk = IOGetInputDataChunk(in, 1);
      TEST_STR(ChkGetElementName(chunk), "radius");
      TEST_INT(ChkGetElementType(chunk), ELM_DOUBLE);
      TEST_INT(ChkGetElementCount(chunk), 3);

      chunk = IOGetInputDataChunk(in, 2);
      TEST_PTR(chunk, NULL);
    }

    IOReadInputData(in);

    {
      TEST_DOUBLE(P[0].x, 1);
      TEST_DOUBLE(P[0].y, 2);
      TEST_DOUBLE(P[0].z, 3);

      TEST_DOUBLE(P[1].x, 2);
      TEST_DOUBLE(P[1].y, 3);
      TEST_DOUBLE(P[1].z, 1);

      TEST_DOUBLE(P[2].x, 3);
      TEST_DOUBLE(P[2].y, 1);
      TEST_DOUBLE(P[2].z, 2);
    }
    {
      TEST_DOUBLE(radius[0], 1.24);
      TEST_DOUBLE(radius[1], 2.34);
      TEST_DOUBLE(radius[2], 3.45);
    }

    IOCloseInputFile(in);
  }

  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}

