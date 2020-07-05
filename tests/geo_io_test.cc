// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#include "unit_test.h"
#include "fj_vector.h"
#include "fj_geo_io.h"
#include <iostream>

using namespace fj;

int main()
{
#if 0
  //const int format_version = 7;
  //const char signature[] = "fjgeo";
#endif
  const int32_t position_count = 8;
  const int32_t normal_count = 6;
  const Vector position[] = {
      Vector(1.1, 2.1, 3.1),
      Vector(2.2, 3.2, 1.2),
      Vector(3.3, 1.3, 2.3)
      };
  const int64_t position_index[] = {0, 1, 2};
  const int64_t position_index_count = 3;

  const Vector normal[] = {
      Vector(1, 0, 0),
      Vector(0, 1, 0),
      Vector(0, 0, 1)
      };
  const int64_t normal_index[] = {0, 1, 2};
  const int64_t normal_index_count = 3;
#if 0
#endif

  {
    Archive ar;

    ar.OpenOutput("archive_test.bin");

    ar.SetPositionData(position, position_count, position_index, position_index_count);
    ar.SetNormalData(normal, normal_count, normal_index, normal_index_count);
#if 0
    ar.SetPositionCount(position_count);
    ar.SetNormalCount(normal_count);

    ar.SetPosition(P);
    ar.AddSignature(signature);
    ar.AddAttribute("position_count", &position_count, 1);
#endif

    ar.Write();
  }
  {
    Archive ar;
    //Vector position[3] = {};

    ar.OpenInput("archive_test.bin");

    TEST_INT(ar.IsFailed(), 0);

    TEST_INT(ar.GetPositionCount(), position_count);

    const std::string name = ar.ReadDataName();
    std::cout << name << "]\n";
    TEST(name == "position_data");

#if N
    // If we want to use an API like this
    //   ar.SetPosition(pos, count, index, index_count)
    // Then we need to allow this API to access to raw data of mesh.
    // And to allocate position data before setting data ref,
    // we have to iterate over data names and check all count data
    // for every attributes.
    while (ar.ReadData()) {
      const std::string name = ar.GetDataName();
      const int64_t count = ar.GetDataCount();

      if (name == "position") {
        mesh.SetVertexCount(count);
        mesh.AddVertexPosition();
        for (int i = 0; i < count; i++) {
          const Vector P = ar.GetData(i);
          mesh.SetVertexPosition(i, P);
        }
      }
      else {
      }
    }
#endif

#if 0
#endif

    //ar.SetOutputPosition(position);

    //ar.Read();
#if 0
    TEST_INT(ar.MatchSignature(signature), 1);

    std::string name;
    name = ar.ReadAttributeName();
    TEST(name == "position_count");

    //int32_t value = 0;
    ar.ReadAttributeData();
#endif
  }
#if 0
  {
    OutputFile *out = IOOpenOutputFile("io_test_file.bin", "PTCD", format_version);
    const Vector P[] = {
        Vector(1, 2, 3),
        Vector(2, 3, 1),
        Vector(3, 1, 2)
        };
    const double radius[] = {
        1.24,
        2.34,
        3.45
        };
    const int npoints = sizeof(P)/sizeof(P[0]);

    IOSetOutputInt     (out, "position_count", &npoints, 1);
    IOEndOutputHeader  (out);
    IOSetOutputVector3 (out, "P",           P,         npoints);
    IOSetOutputDouble  (out, "radius",      radius,    npoints);

    IOWriteOutputHeader(out);
    IOWriteOutputData(out);

    IOCloseOutputFile(out);
  }
  {
    InputFile *in = IOOpenInputFile("io_test_file.bin", "PTCD");
    Vector P[] = {
        Vector(0, 0, 0),
        Vector(0, 0, 0),
        Vector(0, 0, 0)
        };
    double radius[] = {
        0,
        0,
        0
        };
    int npoints = 0;

    TEST_INT(IOGetInputFileFormatVersion(in), format_version);

    IOSetInputInt     (in, "position_count", &npoints, 1);
    IOEndInputHeader  (in);

    IOReadInputHeader(in);
    IOSetInputVector3 (in, "P", P, npoints);
    IOSetInputDouble  (in, "radius", radius, npoints);

    {
      const ChunkData *chunk = NULL;

      TEST_INT(IOGetInputHeaderChunkCount(in), 1);

      chunk = IOGetInputHeaderChunk(in, 0);
      TEST_STR(ChkGetElementName(chunk), "position_count");
      TEST_INT(ChkGetElementType(chunk), ELM_INT);
      TEST_INT(ChkGetElementCount(chunk), 1);
    }
    {
      const ChunkData *chunk = NULL;

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
#endif

  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}
