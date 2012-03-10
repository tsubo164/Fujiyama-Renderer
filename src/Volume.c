/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "Volume.h"
#include "LocalGeometry.h"
#include "Accelerator.h"
#include "Box.h"
#include "Ray.h"
/*
#include "Transform.h"
#include "Numeric.h"
#include "Matrix.h"
#include "Vector.h"
*/
#include <stdlib.h>
#include <float.h>
/*
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
*/

/* volume types */
struct VoxelBuffer {
	float *data;
	int size[3];
};

struct Volume {
	double bounds[6];
	int nbuffers;
};

/* volume interfaces */
static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void volume_bounds(const void *prim_set, int prim_id, double *bounds);

struct Volume *VolNew(void)
{
	struct Volume *volume;

	volume = (struct Volume *) malloc(sizeof(struct Volume));
	if (volume == NULL)
		return NULL;

	BOX3_SET(volume->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	volume->nbuffers = 0;

	/* XXX TEST */
	BOX3_SET(volume->bounds, -1, -1, -1, 1, 1, 1);
	volume->nbuffers = 1;

	return volume;
}

void VolFree(struct Volume *volume)
{
	if (volume == NULL)
		return;

	free(volume);
}

void VolSetupAccelerator(const struct Volume *volume, struct Accelerator *acc)
{
	AccSetTargetGeometry(acc,
			ACC_PRIM_VOLUME,
			volume,
			volume->nbuffers,
			volume->bounds,
			volume_ray_intersect,
			volume_bounds);
}

static int volume_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct Volume *volume = (const struct Volume *) prim_set;
	int hit;
	double boxhit_tmin, boxhit_tmax;

	hit = BoxRayIntersect(volume->bounds, ray->orig, ray->dir, ray->tmin, ray->tmax,
				&boxhit_tmin, &boxhit_tmax);

	if (hit) {
		*t_hit = boxhit_tmin;
	}

	return hit;
}

static void volume_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Volume *volume = (const struct Volume *) prim_set;

	BOX3_COPY(bounds, volume->bounds);
}

#if 0
struct ControlPoint {
	double P[3];
};

struct Bezier3 {
	struct ControlPoint cp[4];
	double width[2];
};

/* bezier volume interfaces */
static double get_bezier3_max_radius(const struct Bezier3 *bezier);
static double get_bezier3_width(const struct Bezier3 *bezier, double t);
static void get_bezier3_bounds(const struct Bezier3 *bezier, double *bounds);
static void get_bezier3(const struct Volume *volume, int prim_id, struct Bezier3 *bezier);
static void eval_bezier3(double *evalP, const struct ControlPoint *cp, double t);
static void derivative_bezier3(double *deriv, const struct ControlPoint *cp, double t);
static void split_bezier3(const struct Bezier3 *bezier,
		struct Bezier3 *left, struct Bezier3 *right);
static int converge_bezier3(const struct Bezier3 *bezier,
		double v0, double vn, int depth,
		double *v_hit, double *P_hit);

/* helper functions */
static void mid_point(double *mid, const double *a, const double *b);
static void cache_split_depth(const struct Volume *volume);
static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon);
static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst);

void *VolAllocateVertex(struct Volume *volume, const char *attr_name, int nverts)
{
	void *ret = NULL;

	if (volume->nverts != 0 && volume->nverts != nverts) {
		/* TODO error handling */
#if 0
		return NULL;
#endif
	}

	if (strcmp(attr_name, "P") == 0) {
		volume->P = VEC3_REALLOC(volume->P, double, nverts);
		ret = volume->P;
	}
	else if (strcmp(attr_name, "width") == 0) {
		volume->width = (double *) realloc(volume->width, 1 * sizeof(double) * nverts);
		ret = volume->width;
	}
	else if (strcmp(attr_name, "Cd") == 0) {
		volume->Cd = VEC3_REALLOC(volume->Cd, float, nverts);
		ret = volume->Cd;
	}
	else if (strcmp(attr_name, "uv") == 0) {
		volume->uv = VEC2_REALLOC(volume->uv, float, nverts);
		ret = volume->uv;
	}

	if (ret == NULL) {
		volume->nverts = 0;
		return NULL;
	}

	volume->nverts = nverts;
	return ret;
}

void *VolAllocateCurve(struct Volume *volume, const char *attr_name, int ncurves)
{
	void *ret = NULL;

	if (volume->ncurves != 0 && volume->ncurves != ncurves) {
		/* TODO error handling */
#if 0
		return NULL;
#endif
	}

	if (strcmp(attr_name, "indices") == 0) {
		volume->indices = (int *) realloc(volume->indices, 1 * sizeof(int) * ncurves);
		ret = volume->indices;
	}

	if (ret == NULL) {
		volume->ncurves = 0;
		return NULL;
	}

	volume->ncurves = ncurves;
	return ret;
}

void VolComputeBounds(struct Volume *volume)
{
	int i;
	double max_radius = 0;

	BOX3_SET(volume->bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; i < volume->ncurves; i++) {
		struct Bezier3 bezier;
		double bezier_bounds[6];
		double bezier_max_radius;

		get_bezier3(volume, i, &bezier);
		get_bezier3_bounds(&bezier, bezier_bounds);
		BoxAddBox(volume->bounds, bezier_bounds);

		bezier_max_radius = get_bezier3_max_radius(&bezier);
		max_radius = MAX(max_radius, bezier_max_radius);
	}

	BOX3_EXPAND(volume->bounds, max_radius);
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

		/* compute v on the volume segment */
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

static void cache_split_depth(const struct Volume *volume)
{
	int i;
	struct Volume *mutable_curve = (struct Volume *) volume;
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

static void get_bezier3(const struct Volume *volume, int prim_id, struct Bezier3 *bezier)
{
	int i0, i1, i2, i3;

	i0 = volume->indices[prim_id];
	i1 = i0 + 1;
	i2 = i0 + 2;
	i3 = i0 + 3;

	VEC3_COPY(bezier->cp[0].P, VEC3_NTH(volume->P, i0));
	VEC3_COPY(bezier->cp[1].P, VEC3_NTH(volume->P, i1));
	VEC3_COPY(bezier->cp[2].P, VEC3_NTH(volume->P, i2));
	VEC3_COPY(bezier->cp[3].P, VEC3_NTH(volume->P, i3));

	bezier->width[0] = volume->width[4*prim_id + 0];
	bezier->width[1] = volume->width[4*prim_id + 3];
}
#endif

