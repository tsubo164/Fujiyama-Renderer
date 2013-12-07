/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "unit_test.h"
#include "fj_vector.h"
#include "fj_iff.h"
#include <string.h>
#include <stdio.h>

int main()
{
  const char filename[] = "iff_test_file.iff";
  {
    const char signature[8] = {128, 'F', 'J', 'G', 'E', 'O', '.', '.'};
    const int64_t point_count = 8;
    const int64_t primitive_count = 12;
    const int32_t point_attribute_count = 2;
    const int32_t primitive_attribute_count = 1;
    {
      IffFile *iff = IffOpen(filename, "wb");
#if n
      IffFile *iff = iff_open(filename, "wb");
#endif

      {
        DataSize geo_chunk = 0;
        DataSize header_chunk = 0;
        DataSize attribute_chunk = 0;

        IffWriteChunkGroupBegin(iff, signature, &geo_chunk);
        {
          IffWriteChunkGroupBegin(iff, "HEADER", &header_chunk);
          {
            IffWriteChunkInt64(iff, "NPT", &point_count, 1);
            IffWriteChunkInt64(iff, "NPR", &primitive_count, 1);

            IffWriteChunkInt32(iff, "NPTATTR", &point_attribute_count, 1);
            IffWriteChunkInt32(iff, "NRTATTR", &primitive_attribute_count, 1);

            IffWriteChunkGroupBegin(iff, "PTALIST", &attribute_chunk);
            {
              IffWriteString(iff, "P");
              IffWriteString(iff, "Cd");
            }
            IffWriteChunkGroupEnd(iff, attribute_chunk);

            IffWriteChunkGroupBegin(iff, "PRALIST", &attribute_chunk);
            {
              IffWriteString(iff, "index");
            }
            IffWriteChunkGroupEnd(iff, attribute_chunk);

          }
          IffWriteChunkGroupEnd(iff, header_chunk);
        }
        IffWriteChunkGroupEnd(iff, geo_chunk);
      }

      IffClose(iff);
    }
    {
      char sig[8] = {'\0'};
      int64_t npt = 0;
      int64_t npr = 0;
      int32_t npta = 0;
      int32_t npra = 0;
      char attr_name[128] = {'\0'};
      IffFile *iff = IffOpen(filename, "rb");

      {
        IffChunk chunk[8];

        IffReadNextChunk(iff, &chunk[0]);
        TEST_INT(memcmp(chunk[0].id, signature, 8),  0);

        IffReadChunkGroupBegin(iff, &chunk[0]);
        {

          IffReadNextChunk(iff, &chunk[1]);
          TEST_STR(chunk[1].id, "HEADER");

          IffReadChunkGroupBegin(iff, &chunk[1]);
          {
            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "NPT");

            IffReadInt64(iff, &npt, 1);
            TEST_INT(npt, point_count);

            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "NPR");

            IffReadInt64(iff, &npr, 1);
            TEST_INT(npr, primitive_count);

            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "NPTATTR");

            IffReadInt32(iff, &npta, 1);
            TEST_INT(npta, point_attribute_count);

            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "NRTATTR");

            IffReadInt32(iff, &npra, 1);
            TEST_INT(npra, primitive_attribute_count);

            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "PTALIST");

            IffReadChunkGroupBegin(iff, &chunk[2]);
            {
              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "P");

              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "Cd");

              TEST_INT(IffEndOfChunk(iff, &chunk[2]), 1);
            }
            IffReadChunkGroupEnd(iff, &chunk[2]);

            IffReadNextChunk(iff, &chunk[2]);
            TEST_STR(chunk[2].id, "PRALIST");

            IffReadChunkGroupBegin(iff, &chunk[2]);
            {
              TEST_INT(IffEndOfChunk(iff, &chunk[2]), 0);

              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "index");
            }
            IffReadChunkGroupEnd(iff, &chunk[2]);

            TEST_INT(IffEndOfChunk(iff, &chunk[2]), 1);

          }
          IffReadChunkGroupEnd(iff, &chunk[1]);

          TEST_INT(IffEndOfChunk(iff, &chunk[1]), 1);

        }
        IffReadChunkGroupEnd(iff, &chunk[0]);

        TEST_INT(IffEndOfChunk(iff, &chunk[0]), 1);

      }

      IffClose(iff);
    }
  }

  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}
