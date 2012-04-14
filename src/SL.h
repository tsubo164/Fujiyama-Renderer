/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SL_H
#define SL_H

#ifdef __cplusplus
extern "C" {
#endif

struct ObjectInstance;
struct ObjectGroup;

enum RayContext {
	CXT_CAMERA_RAY = 0,
	CXT_SHADOW_RAY,
	CXT_REFLECT_RAY,
	CXT_REFRACT_RAY
};

struct TraceContext {
	int ray_context;
	int reflect_depth;
	int refract_depth;
	int max_reflect_depth;
	int max_refract_depth;
	int cast_shadow;

	float opacity_threshold;
	double raymarch_step;
	double raymarch_shadow_step;

	const struct ObjectGroup *trace_target;
};

struct SurfaceInput {
	double P[3];
	double N[3];
	float Cd[3];
	float uv[3];
	float Alpha;

	double Ng[3];
	double I[3];

	double dPds[3];
	double dPdt[3];

	const struct ObjectInstance *shaded_object;
};

struct SurfaceOutput {
	float Cs[3];
	float Os;
};

struct LightOutput {
	float Cl[3];
	float Ol[3];
	double Ln[3];
	double distance;
};

extern double SlSmoothStep(double x, double edge0, double edge1);

extern double SlFresnel(const double *I, const double *N, double ior);
extern double SlPhong(const double *I, const double *N, const double *L, double roughness);

extern void SlReflect(const double *I, const double *N, double *R);
extern void SlRefract(const double *I, const double *N, double ior, double *T);

extern int SlTrace(const struct TraceContext *cxt,
		const double *ray_orig, const double *ray_dir,
		double ray_tmin, double ray_tmax, float *out_color);

extern struct TraceContext SlCameraContext(const struct ObjectGroup *target);
extern struct TraceContext SlReflectContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj);
extern struct TraceContext SlRefractContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj);
extern struct TraceContext SlShadowContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj);

extern int SlIlluminace(const struct TraceContext *cxt, int light_id,
		const double *Ps, const double *axis, float angle,
		const struct SurfaceInput *in, struct LightOutput *out);

extern int SlGetLightCount(const struct SurfaceInput *in);
extern void SlGetLightDirection(const struct SurfaceInput *in, int light_id,
		const double *P, double *out_light_dir);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

