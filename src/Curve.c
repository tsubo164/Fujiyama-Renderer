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
#include <float.h>
#include <stdio.h>
#include <math.h>

struct ControlPoint {
	double P[3];
};

struct BezierCurve3 {
	struct ControlPoint cp[4];
	double width[2];
};

struct Curve {
	struct ControlPoint ctrl_pts[4];
	double bounds[6];

	struct ControlPoint *cps;
	int *indices;

	int ncps;
	int ncurves;
};

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void curve_bounds(const void *prim_set, int prim_id, double *bounds);

static void eval_bezier_curve3(double *evalP, const struct ControlPoint *cp, double t);
static void derivative_bezier_curve3(double *deriv, const struct ControlPoint *cp, double t);
static void compute_bezier_curve_bounds(const struct ControlPoint *cp, double *bounds);
static void compute_bezier_curve_bounds3(const struct BezierCurve3 *bezier, double *bounds);
static void split_bezier_curve3(const struct BezierCurve3 *bezier,
		struct BezierCurve3 *left, struct BezierCurve3 *right);
static int converge_bezier_curve3(const struct BezierCurve3 *bezier,
		double v0, double vn, int depth,
		double *v_hit, double *P_hit);

static void mid_point(double *mid, const double *a, const double *b);
static int compute_split_depth_limit(const struct ControlPoint *cp, double epsilon);
static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst);

static double get_bezier_cureve3_width(const struct BezierCurve3 *bezier, double t);

struct Curve *CrvNew(void)
{
	struct Curve *curve;

	curve = (struct Curve *) malloc(sizeof(struct Curve));
	if (curve == NULL)
		return NULL;

	curve->cps = NULL;
	curve->indices = NULL;
#if 0
	VEC3_SET(curve->ctrl_pts[0].P, 0, -.75, 0);
	VEC3_SET(curve->ctrl_pts[1].P, 0, -.25, 0);
	VEC3_SET(curve->ctrl_pts[2].P, 0,  .25, 0);
	VEC3_SET(curve->ctrl_pts[3].P, 0,  .75, 0);
#endif
	VEC3_SET(curve->ctrl_pts[0].P, -.5, -.75, -5+0);
	VEC3_SET(curve->ctrl_pts[1].P,  .75, -.75, 0);
	VEC3_SET(curve->ctrl_pts[2].P, -.75, .75, 0);
	VEC3_SET(curve->ctrl_pts[3].P,  .5,  .75, 0);

	compute_bezier_curve_bounds(curve->ctrl_pts, curve->bounds);
	BOX3_EXPAND(curve->bounds, .5);
	curve->ncurves = 1;
	PrintBox3d(curve->bounds);

	return curve;
}

void CrvFree(struct Curve *curve)
{
	if (curve == NULL)
		return;

	if (curve->cps != NULL)
		free(curve->cps);

	if (curve->indices != NULL)
		free(curve->indices);

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

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	struct BezierCurve3 bezier;
	struct Matrix world_to_ray;

	double ttmp = FLT_MAX;
	double v_hit;
	int depth;
	int hit;
	int i;

	compute_world_to_ray_matrix(ray, &world_to_ray);
	for (i = 0; i < 4; i++) {
		VEC3_COPY(bezier.cp[i].P, curve->ctrl_pts[i].P);
		TransformPoint(bezier.cp[i].P, &world_to_ray);
	}
	bezier.width[0] = .4;
	bezier.width[0] = .5;
	bezier.width[1] = .01;
	bezier.width[1] = .5;
	/*
	*/

	depth = compute_split_depth_limit(bezier.cp, .5 / 20);
	depth = MIN(depth, 5);

	hit = converge_bezier_curve3(&bezier, 0, 1, depth, &v_hit, &ttmp);
	if (hit) {
		double P_curve[3];
		*t_hit = ttmp;

		POINT_ON_RAY(isect->P,ray->orig, ray->dir, *t_hit);
		eval_bezier_curve3(P_curve, curve->ctrl_pts, v_hit);
		VEC3_SUB(isect->N, isect->P, P_curve);
		VEC3_NORMALIZE(isect->N);

		derivative_bezier_curve3(isect->T, curve->ctrl_pts, v_hit);
		VEC3_NORMALIZE(isect->T);
		if (v_hit > 0) {
		/*
			printf("[%g, %g, %g]\n", isect->T[0], isect->T[1], isect->T[2]);
			printf("[%g]\n", v_hit);
		*/
		}
	}

	return hit;
}

static void curve_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	BOX3_COPY(bounds, curve->bounds);
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

static int converge_bezier_curve3(const struct BezierCurve3 *bezier,
		double v0, double vn, int depth,
		double *v_hit, double *t_hit)
{
	const struct ControlPoint *cp = bezier->cp;
	double radius = MAX(bezier->width[0], bezier->width[1]) * .5;
	double bounds[6];

	compute_bezier_curve_bounds3(bezier, bounds);

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

		radius_w = .5 * get_bezier_cureve3_width(bezier, w);
		/* compare x-y distance */
		eval_bezier_curve3(vP, cp, w);
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
		struct BezierCurve3 bezier_left;
		struct BezierCurve3 bezier_right;
		int hit_left, hit_right;
		double t_left, t_right;
		double v_left, v_right;

		split_bezier_curve3(bezier, &bezier_left, &bezier_right);

		t_left  = FLT_MAX;
		t_right = FLT_MAX;
		hit_left  = converge_bezier_curve3(&bezier_left,  v0, vm, depth-1, &v_left,  &t_left);
		hit_right = converge_bezier_curve3(&bezier_right, vm, vn, depth-1, &v_right, &t_right);

#if 0
		if (hit_left) {
			*t_hit = MIN(*t_hit, t_left);
		}
		if (hit_right) {
			*t_hit = MIN(*t_hit, t_right);
		}
#endif
		if(hit_left || hit_right) {
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

static void eval_bezier_curve3(double *evalP, const struct ControlPoint *cp, double t)
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

static void derivative_bezier_curve3(double *deriv, const struct ControlPoint *cp, double t)
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

static void split_bezier_curve3(const struct BezierCurve3 *bezier,
		struct BezierCurve3 *left, struct BezierCurve3 *right)
{
	double midP[3];
	double midCP[3];

	eval_bezier_curve3(midP, bezier->cp, .5);
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

static void compute_bezier_curve_bounds(const struct ControlPoint *cp, double *bounds)
{
	int i;
	BOX3_SET(bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < 4; i++) {
		BoxAddPoint(bounds, cp[i].P);
	}
}

static void compute_bezier_curve_bounds3(const struct BezierCurve3 *bezier, double *bounds)
{
	int i;
	double maxwidth;
	BOX3_SET(bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < 4; i++) {
		BoxAddPoint(bounds, bezier->cp[i].P);
	}

	maxwidth = .5 * MAX(bezier->width[0], bezier->width[1]);
	BOX3_EXPAND(bounds, maxwidth);
}

static void mid_point(double *mid, const double *a, const double *b)
{
	mid[0] = (a[0] + b[0]) * .5;
	mid[1] = (a[1] + b[1]) * .5;
	mid[2] = (a[2] + b[2]) * .5;
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

static double get_bezier_cureve3_width(const struct BezierCurve3 *bezier, double t)
{
	return LERP(bezier->width[0], bezier->width[1], t);
}

#if 0
void CrvSetPosition(struct Curve *curve, double xpos, double ypos, double zpos)
{
	curve->position[0] = xpos;
	curve->position[1] = ypos;
	curve->position[2] = zpos;
}

void CrvSetColor(struct Curve *curve, float r, float g, float b)
{
	curve->color[0] = r;
	curve->color[1] = g;
	curve->color[2] = b;
}

void CrvSetIntensity(struct Curve *curve, double intensity)
{
	curve->intensity = intensity;
}

const double *CrvGetPosition(const struct Curve *curve)
{
	return curve->position;
}

void CrvIlluminate(const struct Curve *curve, const double *Ps, float *Cl)
{
	Cl[0] = curve->intensity * curve->color[0];
	Cl[1] = curve->intensity * curve->color[1];
	Cl[2] = curve->intensity * curve->color[2];
}
#endif

