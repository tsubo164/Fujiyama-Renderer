/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef TEST_H
#define TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#define TEST(expr) \
	do { if (expr) {\
	    TestPass( #expr, __FILE__, __LINE__ ); \
	} else { \
	    TestFail( #expr, __FILE__, __LINE__ ); \
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

