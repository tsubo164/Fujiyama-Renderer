// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_bvh_accelerator.h"
#include "fj_intersection.h"
#include "fj_primitive_set.h"
#include "fj_accelerator.h"
#include "fj_numeric.h"
#include "fj_box.h"
#include "fj_ray.h"

#include <algorithm>
#include <utility>
#include <vector>
#include <stack>
#include <cassert>

namespace fj {

static const char ACCELERATOR_NAME[] = "BVH";

enum {
  HIT_NONE = 0,
  HIT_LEFT = 1,
  HIT_RIGHT = 2,
  HIT_BOTH = 3
};

class Primitive {
public:
  Primitive() : bounds(), centroid(), index(0) {}
  ~Primitive() {}

  Box bounds;
  Vector centroid;
  int index;
};

class BVHNode {
public:
  BVHNode() : left(NULL), right(NULL), bounds(), prim_id(-1) {}
  ~BVHNode() {}

  bool is_leaf() const
  {
    return (
      left == NULL &&
      right == NULL &&
      prim_id != -1);
  }

  BVHNode *left;
  BVHNode *right;
  Box bounds;
  int prim_id;
};

static bool intersect_bvh_recursive(const PrimitiveSet *primset,
    const BVHNode *node, const Ray &ray, Real time,
    Intersection *isect);
static bool intersect_bvh_loop(const PrimitiveSet *primset,
    const BVHNode *root, const Ray &ray, Real time,
    Intersection *isect);

static BVHNode *new_bvhnode();
static void free_bvhnode_recursive(BVHNode *node);
static BVHNode *build_bvh(Primitive **prims, int begin, int end, int axis);
static int find_median(Primitive **prims, int begin, int end, int axis);

BVHAccelerator::BVHAccelerator() : root(NULL)
{
}

BVHAccelerator::~BVHAccelerator()
{
  free_bvhnode_recursive(root);
}

int BVHAccelerator::build()
{
  const PrimitiveSet *primset = GetPrimitiveSet();
  const int NPRIMS = primset->GetPrimitiveCount();

  if (NPRIMS == 0) {
    // TODO is NPRIMS == 0 error?
    return -1;
  }

  std::vector<Primitive> prims(NPRIMS);
  std::vector<Primitive*> primptrs(NPRIMS, NULL);

  for (int i = 0; i < NPRIMS; i++) {
    primset->GetPrimitiveBounds(i, &prims[i].bounds);
    prims[i].centroid = prims[i].bounds.Centroid();
    prims[i].index = i;

    primptrs[i] = &prims[i];
  }

  root = build_bvh(&primptrs[0], 0, NPRIMS, 0);
  if (root == NULL) {
    // TODO NODE COULD BE NULL IF PRIMITIVE IS EMPTY. MIGHT BE BETTER CHANGE
    return -1;
  }

  return 0;
}

bool BVHAccelerator::intersect(const Ray &ray, Real time, Intersection *isect) const
{
  const PrimitiveSet *primset = GetPrimitiveSet();

  if (1)
    return intersect_bvh_loop(primset, root, ray, time, isect);
  else
    return intersect_bvh_recursive(primset, root, ray, time, isect);
}

const char *BVHAccelerator::get_name() const
{
  return ACCELERATOR_NAME;
}

static bool intersect_bvh_recursive(const PrimitiveSet *primset,
    const BVHNode *node, const Ray &ray, Real time,
    Intersection *isect)
{
  // TODO NODE COULD BE NULL IF PRIMITIVE IS EMPTY. MIGHT BE BETTER CHANGE
  if (node == NULL)
    return false;

  Real boxhit_tmin;
  Real boxhit_tmax;
  const bool hit = BoxRayIntersect(node->bounds,
      ray.orig, ray.dir, ray.tmin, ray.tmax,
      &boxhit_tmin, &boxhit_tmax);

  if (!hit) {
    return false;
  }

  if (node->is_leaf()) {
    return primset->RayIntersect(node->prim_id, ray, time, isect);
  }

  Intersection isect_left, isect_right;
  const bool hit_left  = intersect_bvh_recursive(primset, node->left,  ray, time, &isect_left);
  const bool hit_right = intersect_bvh_recursive(primset, node->right, ray, time, &isect_right);

  if (isect_left.t_hit < ray.tmin)
    isect_left.t_hit = REAL_MAX;
  if (isect_right.t_hit < ray.tmin)
    isect_right.t_hit = REAL_MAX;

  if (isect_left.t_hit < isect_right.t_hit) {
    *isect = isect_left;
  } else if (isect_right.t_hit < isect_left.t_hit) {
    *isect = isect_right;
  }

  return (hit_left || hit_right);
}

static bool intersect_bvh_loop(const PrimitiveSet *primset,
    const BVHNode *root, const Ray &ray, Real time,
    Intersection *isect)
{
  bool hit = false;
  const BVHNode *node = root;
  std::stack<BVHNode*> stack;

  // TODO NODE COULD BE NULL IF PRIMITIVE IS EMPTY. MIGHT BE BETTER CHANGE
  if (node == NULL)
    return false;

  Intersection isect_candidates[2];
  Intersection *isect_min = &isect_candidates[0];
  Intersection *isect_tmp = &isect_candidates[1];

  for (;;) {
    if (node->is_leaf()) {
      const bool hittmp = primset->RayIntersect(node->prim_id, ray, time, isect_tmp);
      if (hittmp && isect_tmp->t_hit < isect_min->t_hit) {
        std::swap(isect_min, isect_tmp);
        hit = hittmp;
      }

      if (stack.empty())
        goto loop_exit;
      node = stack.top();
      stack.pop();
      continue;
    }

    Real boxhit_tmin, boxhit_tmax;
    const bool hit_left = BoxRayIntersect(node->left->bounds,
        ray.orig, ray.dir, ray.tmin, ray.tmax,
        &boxhit_tmin, &boxhit_tmax);

    const bool hit_right = BoxRayIntersect(node->right->bounds,
        ray.orig, ray.dir, ray.tmin, ray.tmax,
        &boxhit_tmin, &boxhit_tmax);

    int whichhit = HIT_NONE;
    whichhit |= hit_left  ? HIT_LEFT :  HIT_NONE;
    whichhit |= hit_right ? HIT_RIGHT : HIT_NONE;

    switch (whichhit) {
    case HIT_NONE:
      if (stack.empty())
        goto loop_exit;
      node = stack.top();
      stack.pop();
      break;

    case HIT_LEFT:
      node = node->left;
      break;

    case HIT_RIGHT:
      node = node->right;
      break;

    case HIT_BOTH:
      stack.push(node->right);
      node = node->left;
      break;

    default:
      assert(!"invalid whichhit");
      break;
    }
  }
loop_exit:

  if (hit) {
    *isect = *isect_min;
  }

  return hit;
}

// Compares an axis component of primitive centroid for std::sort.
template<int Axis>
class CentroidLess {
public:
  bool operator()(Primitive *a, Primitive *b) const
  { 
    return a->centroid[Axis] < b->centroid[Axis];
  }
};

static BVHNode *build_bvh(Primitive **primptrs, int begin, int end, int axis)
{
  BVHNode *node = new_bvhnode();

  if (end - begin == 1) {
    node->prim_id = primptrs[begin]->index;
    node->bounds  = primptrs[begin]->bounds;
    return node;
  }

  Primitive **prim_begin = primptrs + begin;
  Primitive **prim_end   = primptrs + end;

  switch (axis) {
    case 0:
      std::sort(prim_begin, prim_end, CentroidLess<0>());
      break;
    case 1:
      std::sort(prim_begin, prim_end, CentroidLess<1>());
      break;
    case 2:
      std::sort(prim_begin, prim_end, CentroidLess<2>());
      break;
    default:
      assert(!"invalid axis");
      break;
  }

  const int median = find_median(primptrs, begin, end, axis);
  const int new_axis = (axis + 1) % 3;

  node->left  = build_bvh(primptrs, begin, median, new_axis);
  if (node->left == NULL)
    return NULL;

  node->right = build_bvh(primptrs, median, end, new_axis);
  if (node->right == NULL)
    return NULL;

  node->bounds = node->left->bounds;
  node->bounds.AddBox(node->right->bounds);

  return node;
}

static BVHNode *new_bvhnode()
{
  return new BVHNode();
}

static void free_bvhnode_recursive(BVHNode *node)
{
  if (node == NULL)
    return;

  free_bvhnode_recursive(node->left);
  free_bvhnode_recursive(node->right);

  delete node;
}

static int find_median(Primitive **prims, int begin, int end, int axis)
{
  assert(axis >= 0 && axis <= 2);

  int low  = begin;
  int high = end - 1;
  int mid  = -1;
  const Real key = (prims[low]->centroid[axis] + prims[high]->centroid[axis]) / 2;

  while (low != mid) {
    mid = (low + high) / 2;
    if (key < prims[mid]->centroid[axis])
      high = mid;
    else if (prims[mid]->centroid[axis] < key)
      low = mid;
    else
      break;
  }

  return mid + 1;
}

} // namespace xxx
