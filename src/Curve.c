/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Curve.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Transform.h"
#include "Numeric.h"
#include "Matrix.h"
#include "Vector.h"
#include "Ray.h"
#include "Box.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

struct ControlPoint {
	double P[3];
};

struct Bezier3 {
	struct ControlPoint cp[4];
	double width[2];
};

/* curve interfaces */
static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void curve_bounds(const void *prim_set, int prim_id, double *bounds);

/* bezier curve interfaces */
static double get_bezier3_max_radius(const struct Bezier3 *bezier);
static double get_bezier3_width(const struct Bezier3 *bezier, double t);
static void get_bezier3_bounds(const struct Bezier3 *bezier, double *bounds);
static void get_bezier3(const struct Curve *curve, int prim_id, struct Bezier3 *bezier);
static void eval_bezier3(double *evalP, const struct ControlPoint *cp, double t);
static void derivative_bezier3(double *deriv, const struct ControlPoint *cp, double t);
static void split_bezier3(const struct Bezier3 *bezier,
		struct Bezier3 *left, struct Bezier3 *right);
static int converge_bezier3(const struct Bezier3 *bezier,
		double v0, double vn, int depth,
		double *v_hit, double *P_hit);

/* helper functions */
static void mid_point(double *mid, const double *a, const double *b);
static void cache_split_depth(const struct Curve *curve);
static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon);
static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst);

struct Curve *CrvNew(void)
{
	struct Curve *curve;

	curve = (struct Curve *) malloc(sizeof(struct Curve));
	if (curve == NULL)
		return NULL;

	curve->P = NULL;
	curve->width = NULL;
	curve->Cd = NULL;
	curve->uv = NULL;
	curve->indices = NULL;
	BOX3_SET(curve->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	curve->nverts = 0;
	curve->ncurves = 0;

	curve->split_depth = NULL;

	return curve;
}

void CrvFree(struct Curve *curve)
{
	if (curve == NULL)
		return;

	free(curve->P);
	free(curve->width);
	free(curve->Cd);
	free(curve->uv);
	free(curve->indices);
	free(curve->split_depth);

	free(curve);
}

void CrvSetupAccelerator(const struct Curve *curve, struct Accelerator *acc)
{
	AccSetTargetGeometry(acc,
			curve,
			curve->ncurves,
			curve->bounds,
			curve_ray_intersect,
			curve_bounds);
}

void *CrvAllocateVertex(struct Curve *curve, const char *attr_name, int nverts)
{
	void *ret = NULL;

	if (curve->nverts != 0 && curve->nverts != nverts) {
		/* TODO error handling */
#if 0
		return NULL;
#endif
	}

	if (strcmp(attr_name, "P") == 0) {
		curve->P = VEC3_REALLOC(curve->P, double, nverts);
		ret = curve->P;
	}
	else if (strcmp(attr_name, "width") == 0) {
		curve->width = (double *) realloc(curve->width, 1 * sizeof(double) * nverts);
		ret = curve->width;
	}
	else if (strcmp(attr_name, "Cd") == 0) {
		curve->Cd = VEC3_REALLOC(curve->Cd, float, nverts);
		ret = curve->Cd;
	}
	else if (strcmp(attr_name, "uv") == 0) {
		curve->uv = VEC2_REALLOC(curve->uv, float, nverts);
		ret = curve->uv;
	}

	if (ret == NULL) {
		curve->nverts = 0;
		return NULL;
	}

	curve->nverts = nverts;
	return ret;
}

void *CrvAllocateCurve(struct Curve *curve, const char *attr_name, int ncurves)
{
	void *ret = NULL;

	if (curve->ncurves != 0 && curve->ncurves != ncurves) {
		/* TODO error handling */
#if 0
		return NULL;
#endif
	}

	if (strcmp(attr_name, "indices") == 0) {
		curve->indices = (int *) realloc(curve->indices, 1 * sizeof(int) * ncurves);
		ret = curve->indices;
	}

	if (ret == NULL) {
		curve->ncurves = 0;
		return NULL;
	}

	curve->ncurves = ncurves;
	return ret;
}

void CrvComputeBounds(struct Curve *curve)
{
	int i;
	double max_radius = 0;

	BOX3_SET(curve->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; i < curve->ncurves; i++) {
		struct Bezier3 bezier;
		double bezier_bounds[6];
		double bezier_max_radius;

		get_bezier3(curve, i, &bezier);
		get_bezier3_bounds(&bezier, bezier_bounds);
		BoxAddBox(curve->bounds, bezier_bounds);

		bezier_max_radius = get_bezier3_max_radius(&bezier);
		max_radius = MAX(max_radius, bezier_max_radius);
	}

	BOX3_EXPAND(curve->bounds, max_radius);
}

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	struct Bezier3 bezier;
	struct Matrix world_to_ray;

	/* for scaled ray */
	struct Ray nml_ray;
	double ray_scale;

	double ttmp = FLT_MAX;
	double v_hit;
	int depth;
	int hit;
	int i;

	nml_ray = *ray;
	ray_scale = VEC3_LEN(nml_ray.dir);
	VEC3_DIV_ASGN(nml_ray.dir, ray_scale);

	get_bezier3(curve, prim_id, &bezier);
	if (curve->split_depth == NULL) {
		cache_split_depth(curve);
	}
	depth = curve->split_depth[prim_id];

	compute_world_to_ray_matrix(&nml_ray, &world_to_ray);
	for (i = 0; i < 4; i++) {
		TransformPoint(bezier.cp[i].P, &world_to_ray);
	}

	hit = converge_bezier3(&bezier, 0, 1, depth, &v_hit, &ttmp);
	if (hit) {
		struct Bezier3 original;
		float *Cd_curve0;
		float *Cd_curve1;

		/* P */
		*t_hit = ttmp / ray_scale;
		POINT_ON_RAY(isect->P,ray->orig, ray->dir, *t_hit);

		/* dPdt */
		get_bezier3(curve, prim_id, &original);
		derivative_bezier3(isect->dPdt, original.cp, v_hit);

		/* Cd */
		Cd_curve0 = VEC3_NTH(curve->Cd, curve->indices[prim_id]);
		Cd_curve1 = VEC3_NTH(curve->Cd, curve->indices[prim_id]+2);
		VEC3_LERP(isect->Cd, v_hit, Cd_curve0, Cd_curve1);
	}

	return hit;
}

static void curve_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	struct Bezier3 bezier;

	get_bezier3(curve, prim_id, &bezier);
	get_bezier3_bounds(&bezier, bounds);
}

static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst)
{
	double d, d_inv;
	double ox, oy, oz;
	double lx, ly, lz;
	struct Matrix translate, rotate;

	VEC3_GET(ox, oy, oz, ray->orig);
	VEC3_GET(lx, ly, lz, ray->dir);

	d = sqrt(lx*lx + lz*lz);
	if (d == 0) {
		/* TODO handle d == 0 */
	}
	d_inv = 1. / d;

	MatSet(&translate,
			1, 0, 0, -ox,
			0, 1, 0, -oy,
			0, 0, 1, -oz,
			0, 0, 0, 1);
	MatSet(&rotate,
			lz*d_inv, 0, -lx*d_inv, 0,
			-lx*ly*d_inv, d, -ly*lz*d_inv, 0,
			lx, ly, lz, 0,
			0, 0, 0, 1);

	MatMultiply(dst, &rotate, &translate);
}

/* Based on this algorithm:
   Koji Nakamaru and Yoshio Ono, RAY TRACING FOR CURVES PRIMITIVE, WSCG 2002.
   */
static int converge_bezier3(const struct Bezier3 *bezier,
		double v0, double vn, int depth,
		double *v_hit, double *t_hit)
{
	const struct ControlPoint *cp = bezier->cp;
	const double radius = get_bezier3_max_radius(bezier);
	double bounds[6];

	get_bezier3_bounds(bezier, bounds);

	if (bounds[0] >= radius || bounds[3] <= -radius ||
		bounds[1] >= radius || bounds[4] <= -radius ||
		bounds[2] >= *t_hit || bounds[5] <= 1e-6) {
		return 0;
	}

	if (depth == 0) {
		double dir[3];
		double dP0[3];
		double dPn[3];
		double vP[3];
		double v, w;
		double radius_w;

		VEC3_SUB(dir, cp[3].P, cp[0].P);
		VEC3_SUB(dP0, cp[1].P, cp[0].P);

		/* not VEC3_DOT */
		if (VEC2_DOT(dir, dP0) < 0) {
			VEC3_MUL_ASGN(dP0, -1);
		}
		/* not VEC3_DOT */
		if (-1 * VEC2_DOT(dP0, cp[0].P) < 0) {
			return 0;
		}

		VEC3_SUB(dPn, cp[3].P, cp[2].P);

		/* not VEC3_DOT */
		if (VEC2_DOT(dir, dPn) < 0) {
			VEC3_MUL_ASGN(dPn, -1);
		}
		/* not VEC3_DOT */
		if (VEC2_DOT(dPn, cp[3].P) < 0) {
			return 0;
		}

		/* compute w on the line segment */
		w = dir[0] * dir[0] + dir[1] * dir[1];
		if (ABS(w) < 1e-6) {
			return 0;
		}
		w = -(cp[0].P[0] * dir[0] + cp[0].P[1] * dir[1]) / w;
		w = CLAMP(w, 0, 1);

		/* compute v on the curve segment */
		v = v0 * (1-w) + vn * w;

		radius_w = .5 * get_bezier3_width(bezier, w);
		/* compare x-y distance */
		eval_bezier3(vP, cp, w);
		if (vP[0] * vP[0] + vP[1] * vP[1] >= radius_w * radius_w) {
			return 0;
		}

		/* compare z distance */
		if (vP[2] <= 1e-6 || *t_hit < vP[2]) {
			return 0;
		}

		/* we found a new intersection */
		*t_hit = vP[2];
		*v_hit = v;

		return 1;
	}

	{
		const double vm = (v0 + vn) * .5;
		struct Bezier3 bezier_left;
		struct Bezier3 bezier_right;
		int hit_left, hit_right;
		double t_left, t_right;
		double v_left, v_right;

		split_bezier3(bezier, &bezier_left, &bezier_right);

		t_left  = FLT_MAX;
		t_right = FLT_MAX;
		hit_left  = converge_bezier3(&bezier_left,  v0, vm, depth-1, &v_left,  &t_left);
		hit_right = converge_bezier3(&bezier_right, vm, vn, depth-1, &v_right, &t_right);

		if (hit_left || hit_right) {
			if (t_left < t_right) {
				*t_hit = t_left;
				*v_hit = v_left;
			} else {
				*t_hit = t_right;
				*v_hit = v_right;
			}
		}

		return hit_left || hit_right;
	}
}

static void eval_bezier3(double *evalP, const struct ControlPoint *cp, double t)
{
	const double u = 1-t;
	const double a = u * u * u;
	const double b = 3 * u * u * t;
	const double c = 3 * u * t * t;
	const double d = t * t * t;

	evalP[0] = a * cp[0].P[0] + b * cp[1].P[0] + c * cp[2].P[0] + d * cp[3].P[0];
	evalP[1] = a * cp[0].P[1] + b * cp[1].P[1] + c * cp[2].P[1] + d * cp[3].P[1];
	evalP[2] = a * cp[0].P[2] + b * cp[1].P[2] + c * cp[2].P[2] + d * cp[3].P[2];
}

static void derivative_bezier3(double *deriv, const struct ControlPoint *cp, double t)
{
	const double u = 1-t;
	const double a = 2 * u * u;
	const double b = 4 * u * t;
	const double c = 2 * t * t;
	struct ControlPoint CP[3];

	VEC3_SUB(CP[0].P, cp[1].P, cp[0].P);
	VEC3_SUB(CP[1].P, cp[2].P, cp[1].P);
	VEC3_SUB(CP[2].P, cp[3].P, cp[2].P);

	deriv[0] = a * CP[0].P[0] + b * CP[1].P[0] + c * CP[2].P[0];
	deriv[1] = a * CP[0].P[1] + b * CP[1].P[1] + c * CP[2].P[1];
	deriv[2] = a * CP[0].P[2] + b * CP[1].P[2] + c * CP[2].P[2];
}

static void split_bezier3(const struct Bezier3 *bezier,
		struct Bezier3 *left, struct Bezier3 *right)
{
	double midP[3];
	double midCP[3];

	eval_bezier3(midP, bezier->cp, .5);
	mid_point(midCP, bezier->cp[1].P, bezier->cp[2].P);

	VEC3_COPY(left->cp[0].P, bezier->cp[0].P);
	mid_point(left->cp[1].P, bezier->cp[0].P, bezier->cp[1].P);
	mid_point(left->cp[2].P, left->cp[1].P, midCP);
	VEC3_COPY(left->cp[3].P, midP);

	VEC3_COPY(right->cp[3].P, bezier->cp[3].P);
	mid_point(right->cp[2].P, bezier->cp[3].P, bezier->cp[2].P);
	mid_point(right->cp[1].P, right->cp[2].P, midCP);
	VEC3_COPY(right->cp[0].P, midP);

	left->width[0] = bezier->width[0];
	left->width[1] = (bezier->width[0] + bezier->width[1]) * .5;
	right->width[0] = left->width[1];
	right->width[1] = bezier->width[1];
}

static void mid_point(double *mid, const double *a, const double *b)
{
	mid[0] = (a[0] + b[0]) * .5;
	mid[1] = (a[1] + b[1]) * .5;
	mid[2] = (a[2] + b[2]) * .5;
}

static void cache_split_depth(const struct Curve *curve)
{
	int i;
	struct Curve *mutable_curve = (struct Curve *) curve;
	assert(mutable_curve->split_depth == NULL);


	mutable_curve->split_depth = (int *) malloc(sizeof(int) * mutable_curve->ncurves);
	for (i = 0; i < mutable_curve->ncurves; i++) {
		struct Bezier3 bezier;
		int depth;

		get_bezier3(mutable_curve, i, &bezier);
		depth = compute_split_depth_limit(bezier.cp, 2*get_bezier3_max_radius(&bezier) / 20.);
		depth = CLAMP(depth, 1, 5);

		mutable_curve->split_depth[i] = depth;
	}
}

static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon)
{
	int i;
	int r0;
	const int N = 4;
	double L0;

	L0 = -1.;
	for (i = 0; i < N-2; i++) {
		const double x_val = fabs(cp[i].P[0] - 2 * cp[i+1].P[0] + cp[i+2].P[0]);
		const double y_val = fabs(cp[i].P[1] - 2 * cp[i+1].P[1] + cp[i+2].P[1]);
		const double max_val = MAX(x_val, y_val);
		L0 = MAX(L0, max_val);
	}

	r0 = (int) (log(sqrt(2.) * N * (N-1) * L0 / (8. * epsilon)) / log(4.));
	return r0;
}

static double get_bezier3_max_radius(const struct Bezier3 *bezier)
{
	return .5 * MAX(bezier->width[0], bezier->width[1]);
}

static double get_bezier3_width(const struct Bezier3 *bezier, double t)
{
	return LERP(t, bezier->width[0], bezier->width[1]);
}

static void get_bezier3_bounds(const struct Bezier3 *bezier, double *bounds)
{
	int i;
	double max_radius;
	BOX3_SET(bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < 4; i++) {
		BoxAddPoint(bounds, bezier->cp[i].P);
	}

	max_radius = get_bezier3_max_radius(bezier);
	BOX3_EXPAND(bounds, max_radius);
}

static void get_bezier3(const struct Curve *curve, int prim_id, struct Bezier3 *bezier)
{
	int i0, i1, i2, i3;

	i0 = curve->indices[prim_id];
	i1 = i0 + 1;
	i2 = i0 + 2;
	i3 = i0 + 3;

	VEC3_COPY(bezier->cp[0].P, VEC3_NTH(curve->P, i0));
	VEC3_COPY(bezier->cp[1].P, VEC3_NTH(curve->P, i1));
	VEC3_COPY(bezier->cp[2].P, VEC3_NTH(curve->P, i2));
	VEC3_COPY(bezier->cp[3].P, VEC3_NTH(curve->P, i3));

	bezier->width[0] = curve->width[4*prim_id + 0];
	bezier->width[1] = curve->width[4*prim_id + 3];
}

