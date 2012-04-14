/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "SL.h"
#include "VolumeAccelerator.h"
#include "ObjectInstance.h"
#include "Intersection.h"
#include "Accelerator.h"
#include "ObjectGroup.h"
#include "Interval.h"
#include "Numeric.h"
#include "Shader.h"
#include "Vector.h"
#include "Volume.h"
#include "Light.h"
#include "Ray.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

static int has_reached_bounce_limit(const struct TraceContext *cxt);
static void setup_ray(const double *ray_orig, const double *ray_dir,
		double ray_tmin, double ray_tmax,
		struct Ray *ray);
static void setup_surface_input(
		const struct Intersection *isect,
		const struct Ray *ray,
		struct SurfaceInput *in);

static int raymarch_volume(const struct TraceContext *cxt, const struct Ray *ray,
		float *out_rgba);

double SlSmoothStep(double x, double edge0, double edge1)
{
	double t;
	t = (x - edge0) / (edge1 - edge0);
	t = CLAMP(t, 0, 1);
	return t*t*(3 - 2*t);
}

double SlFresnel(const double *I, const double *N, double ior)
{
	double k2;
	double F0;
	double eta;
	double cos;

	/* dot(-I, N) */
	cos = -1 * VEC3_DOT(I, N);
	if (cos > 0) {
		eta = ior;
	} else {
		eta = 1./ior;
		cos *= -1;
	}

	k2 = .0;
	F0 = ((1.-eta) * (1.-eta) + k2) / ((1.+eta) * (1.+eta) + k2);

	return F0 + (1. - F0) * pow(1. - cos, 5.);
}

double SlPhong(const double *I, const double *N, const double *L, double roughness)
{
	double spec;
	double Lrefl[3];

	SlReflect(L, N, Lrefl);

	spec = VEC3_DOT(I, Lrefl);
	spec = MAX(0, spec);
	spec = pow(spec, 1/MAX(.001, roughness));

	return spec;
}

void SlReflect(const double *I, const double *N, double *R)
{
	/* dot(-I, N) */
	double cos = -1 * VEC3_DOT(I, N);

	R[0] = I[0] + 2 * cos * N[0];
	R[1] = I[1] + 2 * cos * N[1];
	R[2] = I[2] + 2 * cos * N[2];
}

void SlRefract(const double *I, const double *N, double ior, double *T)
{
	double radicand;
	double ncoeff;
	double cos1;
	double eta;
	double n[3];

	/* dot(-I, N) */
	cos1 = -1 * VEC3_DOT(I, N);
	if (cos1 < 0) {
		cos1 *= -1;
		eta = 1/ior;
		VEC3_MUL(n, N, -1);
	} else {
		eta = ior;
		VEC3_COPY(n, N);
	}

	radicand = 1 - eta*eta * (1 - cos1*cos1);

	if (radicand < 0.) {
		/* total internal reflection */
		VEC3_MUL(n, N, -1);
		SlReflect(I, N, T);
		return;
	}

	ncoeff = eta * cos1 - sqrt(radicand);

	T[0] = eta * I[0] + ncoeff * n[0];
	T[1] = eta * I[1] + ncoeff * n[1];
	T[2] = eta * I[2] + ncoeff * n[2];
}

int SlTrace(const struct TraceContext *cxt,
		const double *ray_orig, const double *ray_dir,
		double ray_tmin, double ray_tmax, float *out_rgba)
{
	struct Ray ray;
	struct Intersection isect;
	float surface_color[4] = {0};
	float volume_color[4] = {0};
	int hit_surface;
	int hit_volume;

	VEC4_SET(out_rgba, 0, 0, 0, 0);
	if (has_reached_bounce_limit(cxt)) {
		return 0;
	}

	setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);
	hit_surface = AccIntersect(ObjGroupGetSurfaceAccelerator(cxt->trace_target), &ray, &isect);

	/*
	if (cxt->ray_context == CXT_SHADOW_RAY) {
		return hit_surface;
	}
	*/

	if (hit_surface) {
		struct SurfaceInput in;
		struct SurfaceOutput out;

		setup_surface_input(&isect, &ray, &in);
		ShdEvaluate(ObjGetShader(isect.object), cxt, &in, &out);

		out.Os = CLAMP(out.Os, 0, 1);
		VEC4_SET(surface_color, out.Cs[0], out.Cs[1], out.Cs[2], out.Os);

		ray.tmax = isect.t_hit;
	}

	if (cxt->ray_context == CXT_SHADOW_RAY && surface_color[3] > cxt->opacity_threshold) {
		out_rgba[0] = surface_color[0];
		out_rgba[1] = surface_color[1];
		out_rgba[2] = surface_color[2];
		out_rgba[3] = surface_color[3];
		return 1;
	}

	hit_volume = raymarch_volume(cxt, &ray, volume_color);

	out_rgba[0] = volume_color[0] + surface_color[0] * (1-volume_color[3]);
	out_rgba[1] = volume_color[1] + surface_color[1] * (1-volume_color[3]);
	out_rgba[2] = volume_color[2] + surface_color[2] * (1-volume_color[3]);
	out_rgba[3] = volume_color[3] + surface_color[3] * (1-volume_color[3]);

	return hit_surface || hit_volume;

#if 0
	/* original */
	struct Ray ray;
	struct Intersection isect;
	double t_hit = FLT_MAX;
	int hit;

	VEC4_SET(out_rgba, 0, 0, 0, 0);
	if (has_reached_bounce_limit(cxt)) {
		return 0;
	}

	setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);
	hit = AccIntersect(ObjGroupGetSurfaceAccelerator(cxt->trace_target), &ray, &isect, &t_hit);

	if (cxt->ray_context == CXT_SHADOW_RAY) {
		return hit;
	}

	if (hit) {
		struct SurfaceInput in;
		struct SurfaceOutput out;

		setup_surface_input(&isect, &ray, &in);
		ShdEvaluate(ObjGetShader(isect.object), cxt, &in, &out);

		out.Os = CLAMP(out.Os, 0, 1);
		VEC4_SET(out_rgba, out.Cs[0], out.Cs[1], out.Cs[2], out.Os);
	}

	return hit;
#endif

#if 0
	/* original + transparent */
	struct Ray ray;
	struct Ray ray;
	struct Intersection isect;
	double t_hit = FLT_MAX;
	int hit;

	VEC4_SET(out_rgba, 0, 0, 0, 0);
	if (has_reached_bounce_limit(cxt)) {
		return 0;
	}

	setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);

	while (out_rgba[3] < .9995) {
		t_hit = FLT_MAX;
		hit = AccIntersect(ObjGroupGetSurfaceAccelerator(cxt->trace_target), &ray, &isect, &t_hit);

		if (!hit) {
			break;
		}

		/*
		if (cxt->ray_context == CXT_SHADOW_RAY) {
			out_rgba[3] = 1;
			break;
		}
		*/

		if (hit) {
			struct SurfaceInput in;
			struct SurfaceOutput out;

			setup_surface_input(&isect, &ray, &in);
			ShdEvaluate(ObjGetShader(isect.object), cxt, &in, &out);

			out_rgba[0] = out_rgba[0] + out.Cs[0] * (1-out_rgba[3]);
			out_rgba[1] = out_rgba[1] + out.Cs[1] * (1-out_rgba[3]);
			out_rgba[2] = out_rgba[2] + out.Cs[2] * (1-out_rgba[3]);
			out_rgba[3] = out_rgba[3] + CLAMP(out.Os, 0, 1) * (1-out_rgba[3]);
		}

		{
			double next_orig[3];
			POINT_ON_RAY(next_orig, ray.orig, ray.dir, t_hit);
			VEC3_COPY(ray.orig, next_orig);
		}
	}
	out_rgba[3] = CLAMP(out_rgba[3], 0, 1);

	return hit;
#endif
}

struct TraceContext SlCameraContext(const struct ObjectGroup *target)
{
	struct TraceContext cxt;

	cxt.ray_context = CXT_CAMERA_RAY;
	cxt.reflect_depth = 0;
	cxt.refract_depth = 0;
	cxt.max_reflect_depth = 5;
	cxt.max_refract_depth = 5;
	cxt.cast_shadow = 1;
	cxt.trace_target = target;

	cxt.opacity_threshold = .995;
	cxt.raymarch_step = .05;
	cxt.raymarch_shadow_step = .05;

	return cxt;
}

struct TraceContext SlReflectContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj)
{
	struct TraceContext refl_cxt;

	refl_cxt = *cxt;
	refl_cxt.reflect_depth++;
	refl_cxt.ray_context = CXT_REFLECT_RAY;
	refl_cxt.trace_target = ObjGetReflectTarget(obj);

	return refl_cxt;
}

struct TraceContext SlRefractContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj)
{
	struct TraceContext refr_cxt;

	refr_cxt = *cxt;
	refr_cxt.refract_depth++;
	refr_cxt.ray_context = CXT_REFRACT_RAY;
	refr_cxt.trace_target = ObjGetRefractTarget(obj);

	return refr_cxt;
}

struct TraceContext SlShadowContext(const struct TraceContext *cxt,
		const struct ObjectInstance *obj)
{
	struct TraceContext shad_cxt;

	shad_cxt = *cxt;
	shad_cxt.ray_context = CXT_SHADOW_RAY;
	/* turn off the secondary trance on occluding objects */
	shad_cxt.max_reflect_depth = 0;
	shad_cxt.max_refract_depth = 0;
	/* TODO add shadow target */
	shad_cxt.trace_target = ObjGetRefractTarget(obj);

	return shad_cxt;
}

int SlIlluminace(const struct TraceContext *cxt, int light_id,
		const double *Ps, const double *axis, float angle,
		const struct SurfaceInput *in, struct LightOutput *out)
{
	const struct Light **lights;
	const double *light_pos;
	const double *shaded_pos;
	double cosangle;
	double nml_axis[3];
	float light_color[3];

	VEC3_SET(out->Cl, 0, 0, 0);
	if (light_id >= SlGetLightCount(in)) {
		return 0;
	}

	lights = ObjGetLightList(in->shaded_object);
	light_pos = LgtGetPosition(lights[light_id]);
	shaded_pos = in->P;

	VEC3_SUB(out->Ln, light_pos, shaded_pos);
	out->distance = VEC3_LEN(out->Ln);
	if (out->distance > 0) {
		VEC3_DIV_ASGN(out->Ln, out->distance);
	}

	VEC3_COPY(nml_axis, axis);
	VEC3_NORMALIZE(nml_axis);
	cosangle = VEC3_DOT(nml_axis, out->Ln);
	if (cosangle < cos(angle)) {
		return 0;
	}

	LgtIlluminate(lights[light_id], in->P, light_color);
	if (light_color[0] < .0001 &&
		light_color[1] < .0001 &&
		light_color[2] < .0001) {
		return 0;
	}

	if (cxt->cast_shadow) {
		struct TraceContext shad_cxt;
		float C_occl[4];
		int hit;

		shad_cxt = SlShadowContext(cxt, in->shaded_object);
		hit = SlTrace(&shad_cxt, in->P, out->Ln, .0001, out->distance, C_occl);

		if (hit) {
			/*
			return 0;
			*/
			VEC3_MUL_ASGN(light_color, 1-C_occl[3]);
		}
	}

	VEC3_COPY(out->Cl, light_color);
	return 1;
}

int SlGetLightCount(const struct SurfaceInput *in)
{
	return ObjGetLightCount(in->shaded_object);
}

void SlGetLightDirection(const struct SurfaceInput *in, int light_id,
		const double *P, double *out_light_dir)
{
	const struct Light **lights;
	const double *light_pos;
	const double *shaded_pos;

	if (light_id >= SlGetLightCount(in)) {
		VEC3_SET(out_light_dir, 0, 1, 0);
		return;
	}

	lights = ObjGetLightList(in->shaded_object);
	light_pos = LgtGetPosition(lights[light_id]);
	shaded_pos = in->P;

	VEC3_SUB(out_light_dir, light_pos, shaded_pos);
	VEC3_NORMALIZE(out_light_dir);
}

static int has_reached_bounce_limit(const struct TraceContext *cxt)
{
	int current_depth = 0;
	int max_depth = 0;

	switch (cxt->ray_context) {
	case CXT_CAMERA_RAY:
		current_depth = 0;
		max_depth = 1;
		break;
	case CXT_SHADOW_RAY:
		current_depth = 0;
		max_depth = 1;
		break;
	case CXT_REFLECT_RAY:
		current_depth = cxt->reflect_depth;
		max_depth = cxt->max_reflect_depth;
		break;
	case CXT_REFRACT_RAY:
		current_depth = cxt->refract_depth;
		max_depth = cxt->max_refract_depth;
		break;
	default:
		assert(!"invalid ray type");
		break;
	}

	return current_depth > max_depth;
}

static void setup_ray(const double *ray_orig, const double *ray_dir,
		double ray_tmin, double ray_tmax,
		struct Ray *ray)
{
	VEC3_COPY(ray->orig, ray_orig);
	VEC3_COPY(ray->dir, ray_dir);
	ray->tmin = ray_tmin;
	ray->tmax = ray_tmax;
}

static void setup_surface_input(
		const struct Intersection *isect,
		const struct Ray *ray,
		struct SurfaceInput *in)
{
	in->shaded_object = isect->object;
	VEC3_COPY(in->P, isect->P);
	VEC3_COPY(in->N, isect->N);
	VEC3_COPY(in->Cd, isect->Cd);
	VEC3_COPY(in->uv, isect->uv);
	VEC3_COPY(in->I, ray->dir);

	VEC3_COPY(in->dPds, isect->dPds);
	VEC3_COPY(in->dPdt, isect->dPdt);
}

static int raymarch_volume(const struct TraceContext *cxt, const struct Ray *ray,
		float *out_rgba)
{
	const struct VolumeAccelerator *acc;
	struct IntervalList *intervals;
	int hit;

	VEC4_SET(out_rgba, 0, 0, 0, 0);

	intervals = IntervalListNew();
	if (intervals == NULL) {
		/* TODO error handling */
		return 0;
	}

	acc = ObjGroupGetVolumeAccelerator(cxt->trace_target);
	hit = VolumeAccIntersect(acc, ray, intervals);

	if (!hit) {
		return 0;
	}

	{
		double P[3];
		double ray_delta[3];
		double t, t_start, t_delta, t_limit;
		const float opacity_threshold = cxt->opacity_threshold;

		/* t properties */
		switch (cxt->ray_context) {
		case CXT_CAMERA_RAY:
			t_delta = cxt->raymarch_step;
			break;
		case CXT_SHADOW_RAY:
			t_delta = cxt->raymarch_shadow_step;
			break;
		default:
			t_delta = cxt->raymarch_step;
			break;
		}
		t_limit = IntervalListGetMaxT(intervals);
		t_limit = MIN(t_limit, ray->tmax);

		t_start = IntervalListGetMinT(intervals);
		if (t_start < 0) {
			t_start = t_delta;
		}
		else {
			t_start = t_start - fmod(t_start, t_delta) + t_delta;
		}

		POINT_ON_RAY(P, ray->orig, ray->dir, t_start);
		ray_delta[0] = t_delta * ray->dir[0];
		ray_delta[1] = t_delta * ray->dir[1];
		ray_delta[2] = t_delta * ray->dir[2];
		t = t_start;

		/* raymarch */
		while(t <= t_limit && out_rgba[3] < opacity_threshold) {
			const struct Interval *interval = IntervalListGetHead(intervals);
			float color[3] = {0};
			float opacity = 0;

			/* loop over volume candidates at this sample point */
			for (; interval != NULL; interval = interval->next) {
				struct VolumeSample sample;
				ObjGetVolumeSample(interval->object, P, &sample);

				/* merge volume with max density */
				opacity = MAX(opacity, t_delta * sample.density);

				if (cxt->ray_context != CXT_SHADOW_RAY) {
					struct SurfaceInput in;
					struct SurfaceOutput out;

					in.shaded_object = interval->object;
					VEC3_COPY(in.P, P);
					VEC3_SET(in.N, 0, 0, 0);
					ShdEvaluate(ObjGetShader(interval->object), cxt, &in, &out);

					color[0] = out.Cs[0] * opacity;
					color[1] = out.Cs[1] * opacity;
					color[2] = out.Cs[2] * opacity;
				}
			}

			/* composite color */
			out_rgba[0] = out_rgba[0] + color[0] * (1-out_rgba[3]);
			out_rgba[1] = out_rgba[1] + color[1] * (1-out_rgba[3]);
			out_rgba[2] = out_rgba[2] + color[2] * (1-out_rgba[3]);
			out_rgba[3] = out_rgba[3] + CLAMP(opacity, 0, 1) * (1-out_rgba[3]);

			/* advance sample point */
			P[0] += ray_delta[0];
			P[1] += ray_delta[1];
			P[2] += ray_delta[2];
			t += t_delta;
		}
		if (out_rgba[3] >= opacity_threshold) {
			out_rgba[3] = 1;
		}
	}
	out_rgba[3] = CLAMP(out_rgba[3], 0, 1);

	IntervalListFree(intervals);
	return hit;
}

