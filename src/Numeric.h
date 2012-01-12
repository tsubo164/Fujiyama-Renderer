#ifndef NUMERIC_H
#define NUMERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* from math.h */
#define N_E 2.7182818284590452354 /* e */
#define N_LOG2E 1.4426950408889634074 /* log_2 e */
#define N_LOG10E 0.43429448190325182765 /* log_10 e */
#define N_LN2 0.69314718055994530942 /* log_e 2 */
#define N_LN10 2.30258509299404568402 /* log_e 10 */
#define N_PI 3.14159265358979323846 /* pi */
#define N_PI_2 1.57079632679489661923 /* pi/2 */
#define N_PI_4 0.78539816339744830962 /* pi/4 */
#define N_1_PI 0.31830988618379067154 /* 1/pi */
#define N_2_PI 0.63661977236758134308 /* 2/pi */
#define N_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#define N_SQRT2 1.41421356237309504880 /* sqrt(2) */
#define N_SQRT1_2 0.70710678118654752440 /* 1/sqrt(2) */

/* more constants */
#define N_PI_180 0.01745329251994329577 /* pi/180 */
#define N_180_PI 57.2957795130823208768 /* 180/pi */

#define ABS(x_) (((x_)<0)?-(x_):(x_))
#define MIN(x_,y_) ((x_)<(y_)?(x_):(y_))
#define MAX(x_,y_) ((x_)>(y_)?(x_):(y_))
#define CLAMP(x_,lo_,up_) ((x_)<(lo_)?(lo_):((x_)>(up_)?(up_):(x_)))
#define RADIAN(deg_) ((deg_)*N_PI_180)

#define LERP(x_,y_,t_) (((1-(t_))*(x_))+((y_)*(t_)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

