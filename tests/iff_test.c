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
    const char signature[8] = {(char)128, 'F', 'J', 'G', 'E', 'O', '.', '.'};
    const int64_t point_count = 8;
    const int64_t primitive_count = 12;
    const int32_t point_attribute_count = 2;
    const int32_t primitive_attribute_count = 1;
    {
      IffFile *iff = IffOpen(filename, "wb");

      {
        IffChunk geo_chunk;
        IffChunk header_chunk;
        IffChunk attribute_chunk;

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
            IffWriteChunkGroupEnd(iff, &attribute_chunk);

            {
              /* padding test */
              int8_t c = 'f';
              IffWriteChunkInt8(iff, "TEST", &c, 1);
            }

            IffWriteChunkGroupBegin(iff, "PRALIST", &attribute_chunk);
            {
              IffWriteString(iff, "index");
            }
            IffWriteChunkGroupEnd(iff, &attribute_chunk);

            IffWriteChunkGroupBegin(iff, "DTALIST", &attribute_chunk);
            {
            }
            IffWriteChunkGroupEnd(iff, &attribute_chunk);

          }
          IffWriteChunkGroupEnd(iff, &header_chunk);
        }
        IffWriteChunkGroupEnd(iff, &geo_chunk);
      }

      IffClose(iff);
    }
    {
      int64_t npt = 0;
      int64_t npr = 0;
      int32_t npta = 0;
      int32_t npra = 0;
      char attr_name[128] = {'\0'};
      IffFile *iff = IffOpen(filename, "rb");

      {
        IffChunk geo_chunk;
        IffChunk header_chunk;
        IffChunk attribute_chunk;

        IffReadNextChunk(iff, &geo_chunk);
        TEST_INT(memcmp(geo_chunk.id, signature, 8),  0);

        IffPutBackChunk(iff, &geo_chunk);

        IffReadChunkGroupBegin(iff, signature, &geo_chunk);
        TEST_INT(IffChunkMatch(&geo_chunk, signature), 1);
        {

          IffReadNextChunk(iff, &header_chunk);
          TEST_INT(IffChunkMatch(&header_chunk, "HEADER"), 1);

          IffPutBackChunk(iff, &header_chunk);

          IffReadChunkGroupBegin(iff, "HEADER", &header_chunk);
          {
            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "NPT"), 1);

            IffReadInt64(iff, &npt, 1);
            TEST_LONG(npt, point_count);

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "NPR"), 1);

            IffReadInt64(iff, &npr, 1);
            TEST_LONG(npr, primitive_count);

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "NPTATTR"), 1);

            IffReadInt32(iff, &npta, 1);
            TEST_INT(npta, point_attribute_count);

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "NRTATTR"), 1);

            IffReadInt32(iff, &npra, 1);
            TEST_INT(npra, primitive_attribute_count);

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "PTALIST"), 1);

            IffPutBackChunk(iff, &attribute_chunk);

            IffReadChunkGroupBegin(iff, "PTALIST", &attribute_chunk);
            {
              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "P");

              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "Cd");

              TEST_INT(IffEndOfChunk(iff, &attribute_chunk), 1);
            }
            IffReadChunkGroupEnd(iff, &attribute_chunk);

            {
              /* padding test */
              int8_t c = '\0';
              IffReadNextChunk(iff, &attribute_chunk);
              TEST_INT(IffChunkMatch(&attribute_chunk, "TEST"), 1);
              IffReadInt8(iff, &c, 1);
              TEST_INT(c, 'f');
            }

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "PRALIST"), 1);

            IffPutBackChunk(iff, &attribute_chunk);

            IffReadChunkGroupBegin(iff, "PRALIST", &attribute_chunk);
            {
              TEST_INT(IffEndOfChunk(iff, &attribute_chunk), 0);

              IffReadString(iff, attr_name);
              TEST_STR(attr_name, "index");
            }
            IffReadChunkGroupEnd(iff, &attribute_chunk);

            TEST_INT(IffEndOfChunk(iff, &attribute_chunk), 1);

            IffReadNextChunk(iff, &attribute_chunk);
            TEST_INT(IffChunkMatch(&attribute_chunk, "DTALIST"), 1);

            IffPutBackChunk(iff, &attribute_chunk);

            /* ------------------------- */
            {
              const int ret = IffReadNextChildChunk(iff, &header_chunk, &attribute_chunk);
              TEST_INT(ret, 1);
              IffPutBackChunk(iff, &attribute_chunk);
            }
            /* ------------------------- */

            IffReadChunkGroupBegin(iff, "DTALIST", &attribute_chunk);
            {
              TEST_INT(IffEndOfChunk(iff, &attribute_chunk), 1);
            }
            IffReadChunkGroupEnd(iff, &attribute_chunk);

            /* ------------------------- */
            {
              const int ret = IffReadNextChildChunk(iff, &header_chunk, &attribute_chunk);
              TEST_INT(ret, 0);
            }
            /* ------------------------- */
          }
          IffReadChunkGroupEnd(iff, &header_chunk);

          TEST_INT(IffEndOfChunk(iff, &header_chunk), 1);

        }
        IffReadChunkGroupEnd(iff, &geo_chunk);

        TEST_INT(IffEndOfChunk(iff, &geo_chunk), 1);

      }

      IffClose(iff);
    }
  }

  printf("%s: %d/%d/%d: (FAIL/PASS/TOTAL)\n", __FILE__,
      TestGetFailCount(), TestGetPassCount(), TestGetTotalCount());

  return 0;
}
