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

struct ControlPoint {
	double P[3];
};

struct Curve {
	struct ControlPoint ctrl_pts[4];
	double bounds[6];

	double *v0;
	int ncurves;
};

static int curve_ray_intersect(const void *prim_set, int prim_id, const struct Ray *ray,
		struct LocalGeometry *isect, double *t_hit);
static void curve_bounds(const void *prim_set, int prim_id, double *bounds);

static void compute_world_to_ray_matrix(const struct Ray *ray, struct Matrix *dst);
static int converge_bezier_curve(const struct ControlPoint *cp, double v0, double vn,
		int depth, double *t_hit);
static void eval_bezier_curve(double *evalP, const struct ControlPoint *cp, double t);
static void split_bezier_curve(const struct ControlPoint *cp,
		struct ControlPoint *left, struct ControlPoint *right);
static void compute_bezier_curve_bounds(const struct ControlPoint *cp, double *bounds);
static void mid_point(double *mid, const double *a, const double *b);

struct Curve *CrvNew(void)
{
	struct Curve *curve;

	curve = (struct Curve *) malloc(sizeof(struct Curve));
	if (curve == NULL)
		return NULL;

#if 0
	VEC3_SET(curve->ctrl_pts[0].P, -1, 0, 0);
	VEC3_SET(curve->ctrl_pts[1].P,  1, 1, 0);
	VEC3_SET(curve->ctrl_pts[2].P, -1, 2, 0);
	VEC3_SET(curve->ctrl_pts[3].P,  1, 3, 0);
	VEC3_SET(curve->ctrl_pts[0].P, -1, -2, 0);
	VEC3_SET(curve->ctrl_pts[1].P,  1, -1, 0);
	VEC3_SET(curve->ctrl_pts[2].P, -1,  1, 0);
	VEC3_SET(curve->ctrl_pts[3].P,  1,  2, 0);
#endif
	VEC3_SET(curve->ctrl_pts[0].P, 1, -2, 0);
	VEC3_SET(curve->ctrl_pts[1].P, 0, -1, 0);
	VEC3_SET(curve->ctrl_pts[2].P, 0,  1, 0);
	VEC3_SET(curve->ctrl_pts[3].P, 0,  2, 0);


	/*
	BOX3_SET(curve->bounds, -1, 0, 0, 1, 3, 0);
	*/
	compute_bezier_curve_bounds(curve->ctrl_pts, curve->bounds);
	BOX3_EXPAND(curve->bounds, .5);
	curve->ncurves = 1;
	PrintBox3d(curve->bounds);

	puts("Curve new");
#if __linux__
	puts("Yes, Linux");
#endif

	return curve;
}

void CrvFree(struct Curve *curve)
{
	puts("Curve free");
	if (curve == NULL)
		return;
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
	struct ControlPoint projCP[4];

	struct Matrix world_to_ray;
	int depth;
	int hit;
	int i;

	double ttmp = FLT_MAX;

	depth = 4;

	compute_world_to_ray_matrix(ray, &world_to_ray);
	for (i = 0; i < 4; i++) {
		VEC3_COPY(projCP[i].P, curve->ctrl_pts[i].P);
		TransformPoint(projCP[i].P, &world_to_ray);
		/*
		printf("proj >>> (%g, %g, %g)\n", projCP[i].P[0], projCP[i].P[1], projCP[i].P[2]);
		*/
	}

	hit = converge_bezier_curve(projCP, 0, 1, depth, &ttmp);
	if (hit) {
		struct Matrix ray_to_world;
		MatInverse(&ray_to_world, &world_to_ray);

		/*
		TransformPoint(isect->P, &ray_to_world);
		TransformVector(isect->N, &ray_to_world);
		VEC3_NORMALIZE(isect->N);

		POINT_ON_RAY(isect->P,ray->orig, ray->dir, ttmp);
		*/
		/*
		printf("*********HOGE***********\n");
		printf("(%g, %g, %g)\n", isect->P[0], isect->P[1], isect->P[2]);
		*/
		*t_hit = ttmp;
	}

	/*
	{
		struct Ray r;
		struct Matrix m;
		double p[3] = {0, 0, 0};
		VEC3_SET(r.orig, 0, 0, 1);
		VEC3_SET(r.dir, 0, 1, -1);
		VEC3_NORMALIZE(r.dir);
		compute_world_to_ray_matrix(&r, &m);
		TransformPoint(p, &m);
		printf("proj >>> (%g, %g, %g)\n", p[0], p[1], p[2]);
	}
	*/
	return hit;
}

static void curve_bounds(const void *prim_set, int prim_id, double *bounds)
{
	const struct Curve *curve = (const struct Curve *) prim_set;
	BOX3_COPY(bounds, curve->bounds);
	puts("+++++++++++++++++++++++++++++++++++++");
	PrintBox3d(bounds);
	puts("+++++++++++++++++++++++++++++++++++++");
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
#if 0
#endif
#if 0
	MatSet(&translate,
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-ox, -oy, -oz, 1);
	MatSet(&rotate,
			lz*d_inv, -lx*ly*d_inv, lx, 0,
			0, d, ly, 0,
			-lx*d_inv, -ly*lz*d_inv, lz, 0,
			0, 0, 0, 1);
#endif

	/*
	MatMultiply(dst, &translate, &rotate);
	*/
	MatMultiply(dst, &rotate, &translate);
}

static int converge_bezier_curve(const struct ControlPoint *cp, double v0, double vn,
		int depth, double *t_hit)
{
	double radius = .5;
	double bounds[6];

	compute_bezier_curve_bounds(cp, bounds);
	/*
	BOX3_EXPAND(bounds, .5);
	BOX3_EXPAND(bounds, .1);
	*/

	/*
	printf("depth: %d ", depth);
	PrintBox3d(bounds);
	puts("================================");
	*/

	if (bounds[2] >= *t_hit || bounds[5] <= .00001 ||
		bounds[0] >= radius || bounds[3] <= -radius ||
		bounds[1] >= radius || bounds[4] <= -radius) {
		return 0;
	}

	if (depth == 0) {
		double dir[3];
		double dP0[3];
		double dPn[3];
		double vP[3];
		double v, w;

/*
	printf("NEXT DEPTH: %d\n", depth);
*/
		dir[0] = cp[3].P[0] - cp[0].P[0];
		dir[1] = cp[3].P[1] - cp[0].P[1];
		dir[2] = cp[3].P[2] - cp[0].P[2];

		dP0[0] = cp[1].P[0] - cp[0].P[0];
		dP0[1] = cp[1].P[1] - cp[0].P[1];
		dP0[2] = cp[1].P[2] - cp[0].P[2];

		if (VEC3_DOT(dir, dP0) < 0) {
			VEC3_MUL_ASGN(dP0, -1);
		}
		if (-1 * VEC3_DOT(dP0, cp[0].P) < 0) {
			return 0;
		}

		dPn[0] = cp[3].P[0] - cp[2].P[0];
		dPn[1] = cp[3].P[1] - cp[2].P[1];
		dPn[2] = cp[3].P[2] - cp[2].P[2];

		if (VEC3_DOT(dir, dPn) < 0) {
			VEC3_MUL_ASGN(dPn, -1);
		}
		if (VEC3_DOT(dPn, cp[3].P) < 0) {
			return 0;
		}

		/* compute w on the line segment */
		w = dir[0] * dir[0] + dir[1] * dir[1];
		if (w < 1e-6)
			return 0;

		w = -(cp[0].P[0] * dir[0] + cp[0].P[1] * dir[1]) / w;
		w = CLAMP(w, 0, 1);

		/* compute v on the curve segment */
		v = v0 * (1-w) + vn * w;

		/* compare x-y distance */
		eval_bezier_curve(vP, cp, v);
		/*
		printf("(%g, %g, %g)\n", vP[0], vP[1], vP[2]);
		*/
		if (vP[0] * vP[0] + vP[1] * vP[1] >= radius * radius)
			return 0;

		/*
		*/
		/* compare z distance */
		if (vP[2] <= 1e-6 || *t_hit < vP[2])
			return 0;

		/* we found a new intersection */
		*t_hit = vP[2];
		/*
		printf("t_hit: %g\n", *t_hit);
		*/
		return 1;
	}

	{
		const double vm = (v0 + vn) * .5;
		struct ControlPoint cp_left[4];
		struct ControlPoint cp_right[4];
		int hit_left, hit_right;
		double t_left, t_right;

		split_bezier_curve(cp, cp_left, cp_right);

		depth--;

		/*
		hit_left  = converge_bezier_curve(cp_left,  v0, vm, depth, t_hit);
		hit_right = converge_bezier_curve(cp_right, vm, vn, depth, t_hit);
		*/
		t_left = FLT_MAX;
		t_right = FLT_MAX;
		hit_left  = converge_bezier_curve(cp_left,  v0, vm, depth, &t_left);
		hit_right = converge_bezier_curve(cp_right, vm, vn, depth, &t_right);
		if (hit_left || hit_right) {
			*t_hit = MIN(t_left, t_right);
		}

		return hit_left || hit_right;
	}
}

static void eval_bezier_curve(double *evalP, const struct ControlPoint *cp, double t)
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

static void split_bezier_curve(const struct ControlPoint *cp,
		struct ControlPoint *left, struct ControlPoint *right)
{
	double midP[3];
	double midCP[3];

	eval_bezier_curve(midP, cp, .5);
	mid_point(midCP, cp[1].P, cp[2].P);

	VEC3_COPY(left[0].P, cp[0].P);
	mid_point(left[1].P, cp[0].P, cp[1].P);
	mid_point(left[2].P, left[1].P, midCP);
	VEC3_COPY(left[3].P, midP);

	VEC3_COPY(right[3].P, cp[3].P);
	mid_point(right[2].P, cp[2].P, cp[3].P);
	mid_point(right[1].P, midCP, right[2].P);
	VEC3_COPY(right[0].P, midP);
}

static void compute_bezier_curve_bounds(const struct ControlPoint *cp, double *bounds)
{
	int i;
	BOX3_SET(bounds, FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < 4; i++) {
		BoxAddPoint(bounds, cp[i].P);
	}
}

static void mid_point(double *mid, const double *a, const double *b)
{
	mid[0] = (a[0] + b[0]) * .5;
	mid[1] = (a[1] + b[1]) * .5;
	mid[2] = (a[2] + b[2]) * .5;
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

