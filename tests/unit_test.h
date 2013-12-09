/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEST(expr) \
  do { if (expr) {\
      TestPass( #expr, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #expr, __FILE__, __LINE__ ); \
  } } while (0)

#define TEST_INT(a, b) \
  do { if ((a)==(b)) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   %d\n", (a)); \
    printf("*   expected: %d\n", (b)); \
  } } while (0)

#define TEST_LONG(a, b) \
  do { if ((a)==(b)) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   %ld\n", (a)); \
    printf("*   expected: %ld\n", (b)); \
  } } while (0)

#define TEST_FLOAT(a, b) \
  do { if (TestFloatEq((a),(b))) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   %g\n", (a)); \
    printf("*   expected: %g\n", (b)); \
  } } while (0)

#define TEST_DOUBLE(a, b) \
  do { if (TestDoubleEq((a),(b))) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   %g\n", (double)(a)); \
    printf("*   expected: %g\n", (double)(b)); \
  } } while (0)

#define TEST_STR(a, b) \
  do { if (strcmp((a),(b))==0) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   \"%s\"\n", (a)); \
    printf("*   expected: \"%s\"\n", (b)); \
  } } while (0)

#define TEST_PTR(a, b) \
  do { if ((void *)(a) == (void *)(b)) {\
      TestPass( #a" == "#b, __FILE__, __LINE__ ); \
  } else { \
      TestFail( #a" == "#b, __FILE__, __LINE__ ); \
    printf("*   actual:   %p\n", (void *)(a)); \
    printf("*   expected: %p\n", (void *)(b)); \
  } } while (0)

extern void TestPass( const char *expr, const char *file, int line );
extern void TestFail( const char *expr, const char *file, int line );

extern int TestGetPassCount();
extern int TestGetFailCount();
extern int TestGetTotalCount();

extern int TestDoubleEq(double a, double b);
extern int TestFloatEq(float a, float b);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

