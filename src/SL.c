/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "SL.h"
#include "Ray.h"
#include "Light.h"
#include "Shader.h"
#include "Vector.h"
#include "Numeric.h"
#include "Accelerator.h"
#include "LocalGeometry.h"
#include "ObjectInstance.h"

#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

static int has_reached_bounce_limit(const struct TraceContext *cxt);
static void setup_ray(const double *ray_orig, const double *ray_dir,
		double ray_tmin, double ray_tmax,
		struct Ray *ray);
static void setup_surface_input(
		const struct LocalGeometry *local,
		const struct Ray *ray,
		struct SurfaceInput *in);

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
		double ray_tmin, double ray_tmax, float *out_color)
{
	struct Ray ray;
	struct LocalGeometry local;
	double t_hit = FLT_MAX;
	int hit;

	VEC3_SET(out_color, 0, 0, 0);
	if (has_reached_bounce_limit(cxt)) {
		return 0;
	}

	setup_ray(ray_orig, ray_dir, ray_tmin, ray_tmax, &ray);
	hit = AccIntersect(ObjGroupGetAccelerator(cxt->trace_target), &ray, &local, &t_hit);

	if (hit && cxt->ray_context != CXT_SHADOW_RAY) {
		struct SurfaceInput in;
		struct SurfaceOutput out;

		setup_surface_input(&local, &ray, &in);
		ShdEvaluate(ObjGetShader(local.object), cxt, &in, &out);
		VEC3_COPY(out_color, out.Cs);
	}

	return hit;
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
		const struct SurfaceInput *in, struct LightOutput *out)
{
	int hit = 0;

	if (cxt->cast_shadow) {
		struct TraceContext shad_cxt;
		const struct Light **lights;
		const double *light_pos;
		const double *shaded_pos;
		float C_occl[3];

		if (light_id >= SlGetLightCount(in))
			return 0;

		lights = ObjGetLightList(in->shaded_object);
		light_pos = LgtGetPosition(lights[light_id]);
		shaded_pos = in->P;

		VEC3_SUB(out->Ln, light_pos, shaded_pos);
		out->distance = VEC3_LEN(out->Ln);
		if (out->distance > 0) {
			VEC3_DIV_ASGN(out->Ln, out->distance);
		}

		shad_cxt = SlShadowContext(cxt, in->shaded_object);
		hit = SlTrace(&shad_cxt, in->P, out->Ln, .0001, out->distance, C_occl);

		if (hit) {
			VEC3_SET(out->Cl, 0, 0, 0);
		} else {
			LgtIlluminate(lights[light_id], in->P, out->Cl);
		}
	} else {
		/* no shadow */
		const struct Light **lights;
		const double *light_pos;
		const double *shaded_pos;

		if (light_id >= SlGetLightCount(in))
			return 0;

		lights = ObjGetLightList(in->shaded_object);
		light_pos = LgtGetPosition(lights[light_id]);
		shaded_pos = in->P;

		VEC3_SUB(out->Ln, light_pos, shaded_pos);
		out->distance = VEC3_LEN(out->Ln);
		if (out->distance > 0) {
			VEC3_DIV_ASGN(out->Ln, out->distance);
		}
		LgtIlluminate(lights[light_id], in->P, out->Cl);
	}

	return hit;
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
		const struct LocalGeometry *local,
		const struct Ray *ray,
		struct SurfaceInput *in)
{
	in->shaded_object = local->object;
	VEC3_COPY(in->P, local->P);
	VEC3_COPY(in->N, local->N);
	VEC3_COPY(in->Cd, local->Cd);
	VEC3_COPY(in->uv, local->uv);
	VEC3_COPY(in->I, ray->dir);

	VEC3_COPY(in->T, local->T);
}

