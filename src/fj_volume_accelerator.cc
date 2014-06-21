// Copyright (c) 2011-2014 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_volume_accelerator.h"
#include "fj_intersection.h"
#include "fj_interval.h"
#include "fj_volume.h"
#include "fj_ray.h"

#include <vector>
#include <cassert>
#include <cstdio>
#include <cstdlib>

#define EXPAND .0001
#define HALF_EXPAND (.5*EXPAND)

namespace fj {

VolumeAccelerator::VolumeAccelerator() :
    name_(NULL),
    bounds_(),
    has_built_(false),

    volume_set_(NULL),
    num_volumes_(0),
    volume_set_bounds_(),
    VolumeIntersect_(NULL),
    VolumeBounds_(NULL),

    derived_(NULL),
    FreeDerived_(NULL),
    Build_(NULL),
    Intersect_(NULL)
{
}

VolumeAccelerator::~VolumeAccelerator()
{
}

const Box &VolumeAccelerator::GetBounds() const
{
  return bounds_;
}

int VolumeAccelerator::Build()
{
  return build();
}

bool VolumeAccelerator::Intersect(const Ray &ray, double time,
    IntervalList *intervals) const
{
  return intersect(ray, time, intervals);
}

/* -------------------------------------------------------------------------- */
/* VolumeAccelerator */
static int ray_volume_intersect(const VolumeAccelerator *acc, int volume_id,
  double time, const Ray *ray, IntervalList *intervals);
#if 0
static void swap_isect_ptr(IntervalList **isect0, IntervalList **isect1);
#endif

/* -------------------------------------------------------------------------- */
/* VolumeBruteForceAccelerator */
class VolumeBruteForceAccelerator {
public:
  VolumeBruteForceAccelerator() {}
  ~VolumeBruteForceAccelerator() {}

public:
  Volume **volume_list;
  int nvolumes;
};

static VolumeBruteForceAccelerator *new_bruteforce_accel(void);
static void free_bruteforce_accel(VolumeAccelerator *acc);
static int build_bruteforce_accel(VolumeAccelerator *acc);
static int intersect_bruteforce_accel(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals);

/* -------------------------------------------------------------------------- */
/* VolumeBVHAccelerator */
#if 0
enum { BVH_STACKSIZE = 64 };
enum {
  HIT_NONE = 0,
  HIT_LEFT = 1,
  HIT_RIGHT = 2,
  HIT_BOTH = 3
};
#endif

class VolumePrimitive {
public:
  VolumePrimitive() : bounds(), centroid(), index(0) {}
  ~VolumePrimitive() {}

  Box bounds;
  Vector centroid;
  int index;
};

class VolumeBVHNode {
public:
  VolumeBVHNode() : left(NULL), right(NULL), bounds(), volume_id(-1) {}
  ~VolumeBVHNode() {}

  bool is_leaf() const
  {
    return (
      left == NULL &&
      right == NULL &&
      volume_id != -1);
  }

  VolumeBVHNode *left;
  VolumeBVHNode *right;
  Box bounds;
  int volume_id;
};

#if 0
class VolumeBVHNodeStack {
public:
  VolumeBVHNodeStack() {}
  ~VolumeBVHNodeStack() {}

  int depth;
  const VolumeBVHNode *node[BVH_STACKSIZE];
};
#endif

class VolumeBVHAccelerator {
public:
  VolumeBVHAccelerator();
  ~VolumeBVHAccelerator();

public:
  VolumeBVHNode *root_;
};

static VolumeBVHAccelerator *new_bvh_accel(void);
static void free_bvh_accel(VolumeAccelerator *acc);
static int build_bvh_accel(VolumeAccelerator *acc);
static int intersect_bvh_accel(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals);

static int intersect_bvh_recursive(const VolumeAccelerator *acc,
    const VolumeBVHNode *node, double time,
    const Ray *ray, IntervalList *intervals);
#if 0
static int intersect_bvh_loop(const VolumeAccelerator *acc, const VolumeBVHNode *root,
    const Ray *ray, IntervalList *intervals);
#endif

static VolumeBVHNode *new_bvhnode(void);
static void free_bvhnode_recursive(VolumeBVHNode *node);
static VolumeBVHNode *build_bvh(VolumePrimitive **volumes, int begin, int end, int axis);
static int is_bvh_leaf(const VolumeBVHNode *node);
static int find_median(VolumePrimitive **volumes, int begin, int end, int axis);

static int compare_double(double x, double y);
static int volume_compare_x(const void *a, const void *b);
static int volume_compare_y(const void *a, const void *b);
static int volume_compare_z(const void *a, const void *b);

#if 0
static int is_empty(const VolumeBVHNodeStack *stack);
static void push_node(VolumeBVHNodeStack *stack, const VolumeBVHNode *node);
static const VolumeBVHNode *pop_node(VolumeBVHNodeStack *stack);
#endif

VolumeBVHAccelerator::VolumeBVHAccelerator() : root_(NULL)
{
}

VolumeBVHAccelerator::~VolumeBVHAccelerator()
{
  free_bvhnode_recursive(root_);
}

VolumeAccelerator *VolumeAccNew(int accelerator_type)
{
  VolumeAccelerator *acc = new VolumeAccelerator();
  if (acc == NULL)
    return NULL;

  switch (accelerator_type) {
  case VOLACC_BRUTEFORCE:
    acc->derived_ = (char *) new_bruteforce_accel();
    if (acc->derived_ == NULL) {
      VolumeAccFree(acc);
      return NULL;
    }
    acc->FreeDerived_ = free_bruteforce_accel;
    acc->Build_ = build_bruteforce_accel;
    acc->Intersect_ = intersect_bruteforce_accel;
    acc->name_ = "Brute Force";
    break;
  case VOLACC_BVH:
    acc->derived_ = (char *) new_bvh_accel();
    if (acc->derived_ == NULL) {
      VolumeAccFree(acc);
      return NULL;
    }
    acc->FreeDerived_ = free_bvh_accel;
    acc->Build_ = build_bvh_accel;
    acc->Intersect_ = intersect_bvh_accel;
    acc->name_ = "Volume BVH";
    break;
  default:
    assert(!"invalid accelerator type");
    break;
  }

  acc->has_built_ = 0;

  acc->volume_set_ = NULL;
  acc->num_volumes_ = 0;
  acc->VolumeIntersect_ = NULL;
  acc->VolumeBounds_ = NULL;

  BoxReverseInfinite(&acc->bounds_);
  BoxReverseInfinite(&acc->volume_set_bounds_);

  return acc;
}

void VolumeAccFree(VolumeAccelerator *acc)
{
  if (acc == NULL)
    return;
  if (acc->derived_ == NULL)
    return;

  acc->FreeDerived_(acc);
  delete acc;
}

int VolumeAccBuild(VolumeAccelerator *acc)
{
  int err;

  if (acc->has_built_)
    return -1;

  err = acc->Build_(acc);
  if (err)
    return -1;

  acc->has_built_ = 1;
  return 0;
}

int VolumeAccIntersect(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals)
{
  double boxhit_tmin;
  double boxhit_tmax;

  /* check intersection with overall bounds */
  if (!BoxRayIntersect(acc->bounds_, ray->orig, ray->dir, ray->tmin, ray->tmax,
        &boxhit_tmin, &boxhit_tmax)) {
    return 0;
  }

  if (!acc->has_built_) {
    /* dynamic build */
    printf("\nbuilding %s accelerator ...\n", acc->name_);
    VolumeAccBuild((VolumeAccelerator *) acc);
    fflush(stdout);
  }

  return acc->Intersect_(acc, time, ray, intervals);
}

void VolumeAccSetTargetGeometry(VolumeAccelerator *acc,
  const void *volume_set, int num_volumes, const Box *volume_set_bounds,
  VolumeIntersectFunction volume_intersect_function,
  VolumeBoundsFunction volume_bounds_function)
{
  acc->volume_set_ = volume_set;
  acc->num_volumes_ = num_volumes;
  acc->VolumeIntersect_ = volume_intersect_function;
  acc->VolumeBounds_ = volume_bounds_function;
  acc->volume_set_bounds_ = *volume_set_bounds;

  /* accelerator's bounds */
  acc->bounds_ = *volume_set_bounds;
  BoxExpand(&acc->bounds_, EXPAND);
}

/* -------------------------------------------------------------------------- */
/* VolumeBruteForceAccelerator */
static VolumeBruteForceAccelerator *new_bruteforce_accel(void)
{
  VolumeBruteForceAccelerator *bruteforce = new VolumeBruteForceAccelerator();
  if (bruteforce == NULL)
    return NULL;

  bruteforce->volume_list = NULL;
  bruteforce->nvolumes = 0;

  return bruteforce;
}

static void free_bruteforce_accel(VolumeAccelerator *acc)
{
  VolumeBruteForceAccelerator *bruteforce = (VolumeBruteForceAccelerator *) acc->derived_;
  delete bruteforce;
}

static int build_bruteforce_accel(VolumeAccelerator *acc)
{
  return 0;
}

static int intersect_bruteforce_accel(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals)
{
  int hit;
  int i;

  hit = 0;
  for (i = 0; i < acc->num_volumes_; i++) {
    hit += ray_volume_intersect(acc, i, time, ray, intervals);
  }

  return hit > 0;
}

/* -------------------------------------------------------------------------- */
/* VolumeBVHAccelerator */
static VolumeBVHAccelerator *new_bvh_accel(void)
{
  return new VolumeBVHAccelerator();
}

static void free_bvh_accel(VolumeAccelerator *acc)
{
  VolumeBVHAccelerator *bvh = (VolumeBVHAccelerator *) acc->derived_;

  delete bvh;
}

static int build_bvh_accel(VolumeAccelerator *acc)
{
  VolumeBVHAccelerator *bvh = (VolumeBVHAccelerator *) acc->derived_;
  const int NPRIMS = acc->num_volumes_;

  if (NPRIMS == 0) {
    return -1;
  }

  std::vector<VolumePrimitive> volumes(NPRIMS);
  std::vector<VolumePrimitive *> volume_ptr(NPRIMS);

  for (int i = 0; i < NPRIMS; i++) {
    acc->VolumeBounds_(acc->volume_set_, i, &volumes[i].bounds);
    volumes[i].centroid = BoxCentroid(volumes[i].bounds);
    volumes[i].index = i;

    volume_ptr[i] = &volumes[i];
  }

  bvh->root_ = build_bvh(&volume_ptr[0], 0, NPRIMS, 0);
  if (bvh->root_ == NULL) {
    return -1;
  }

  return 0;
}

static int intersect_bvh_accel(const VolumeAccelerator *acc, double time,
    const Ray *ray, IntervalList *intervals)
{
  const VolumeBVHAccelerator *bvh = (const VolumeBVHAccelerator *) acc->derived_;

#if 0
  if (1)
    return intersect_bvh_loop(acc, bvh->root_, ray, intervals);
  else
#endif
    return intersect_bvh_recursive(acc, bvh->root_, time, ray, intervals);
}

static int intersect_bvh_recursive(const VolumeAccelerator *acc,
    const VolumeBVHNode *node, double time,
    const Ray *ray, IntervalList *intervals)
{
  double boxhit_tmin;
  double boxhit_tmax;
  int hit_left, hit_right;
  int hit;

  hit = BoxRayIntersect(node->bounds,
      ray->orig, ray->dir, ray->tmin, ray->tmax,
      &boxhit_tmin, &boxhit_tmax);
  if (!hit) {
    return 0;
  }

  if (is_bvh_leaf(node)) {
    return ray_volume_intersect(acc, node->volume_id, time, ray, intervals);
  }

  hit_left  = intersect_bvh_recursive(acc, node->left,  time, ray, intervals);
  hit_right = intersect_bvh_recursive(acc, node->right, time, ray, intervals);

  return hit_left || hit_right;
}

#if 0
static int intersect_bvh_loop(const VolumeAccelerator *acc, const VolumeBVHNode *root,
    const Ray *ray, IntervalList *intervals)
{
  int hit, hittmp;
  int whichhit;
  int hit_left, hit_right;
  double boxhit_tmin, boxhit_tmax;
  IntervalList isect_candidates[2];
  IntervalList *isect_min, *isect_tmp;

  const VolumeBVHNode *node;
  VolumeBVHNodeStack stack = {0, {NULL}};

  node = root;
  hit = 0;

  isect_min = &isect_candidates[0];
  isect_tmp = &isect_candidates[1];
  isect_min->t_hit = FLT_MAX;

  for (;;) {
    if (is_bvh_leaf(node)) {
      hittmp = ray_volume_intersect(acc, node->volume_id, ray, isect_tmp);
      if (hittmp && isect_tmp->t_hit < isect_min->t_hit) {
        swap_isect_ptr(&isect_min, &isect_tmp);
        hit = hittmp;
      }

      if (is_empty(&stack))
        goto loop_exit;
      node = pop_node(&stack);
      continue;
    }

    hit_left = BoxRayIntersect(node->left->bounds_,
        ray->orig, ray->dir, ray->tmin, ray->tmax,
        &boxhit_tmin, &boxhit_tmax);

    hit_right = BoxRayIntersect(node->right->bounds_,
        ray->orig, ray->dir, ray->tmin, ray->tmax,
        &boxhit_tmin, &boxhit_tmax);

    whichhit = HIT_NONE;
    whichhit |= hit_left  ? HIT_LEFT :  HIT_NONE;
    whichhit |= hit_right ? HIT_RIGHT : HIT_NONE;

    switch (whichhit) {
    case HIT_NONE:
      if (is_empty(&stack))
        goto loop_exit;
      node = pop_node(&stack);
      break;

    case HIT_LEFT:
      node = node->left;
      break;

    case HIT_RIGHT:
      node = node->right;
      break;

    case HIT_BOTH:
      push_node(&stack, node->right);
      node = node->left;
      break;

    default:
      assert(!"invalid whichhit");
      break;
    }
  }
loop_exit:

  if (hit) {
    *intervals = *isect_min;
  }

  return hit;
}
#endif

static VolumeBVHNode *build_bvh(VolumePrimitive **volume_ptr, int begin, int end, int axis)
{
  VolumeBVHNode *node = NULL;
  const int NPRIMS = end - begin;
  int median;
  int new_axis;
  int (*volume_compare)(const void *, const void *);

  node = new_bvhnode();
  if (node == NULL)
    return NULL;

  if (NPRIMS == 1) {
    node->volume_id = volume_ptr[begin]->index;
    node->bounds = volume_ptr[begin]->bounds;
    return node;
  }

  switch (axis) {
    case 0:
      volume_compare = volume_compare_x;
      new_axis = 1;
      break;
    case 1:
      volume_compare = volume_compare_y;
      new_axis = 2;
      break;
    case 2:
      volume_compare = volume_compare_z;
      new_axis = 0;
      break;
    default:
      assert(!"invalid axis");
      break;
  }

  qsort(volume_ptr + begin, NPRIMS, sizeof(VolumePrimitive *), volume_compare);
  median = find_median(volume_ptr, begin, end, axis);

  node->left  = build_bvh(volume_ptr, begin, median, new_axis);
  if (node->left == NULL)
    return NULL;

  node->right = build_bvh(volume_ptr, median, end, new_axis);
  if (node->right == NULL)
    return NULL;

  node->bounds = node->left->bounds;
  BoxAddBox(&node->bounds, node->right->bounds);

  return node;
}

static VolumeBVHNode *new_bvhnode(void)
{
  VolumeBVHNode *node = new VolumeBVHNode();
  if (node == NULL)
    return NULL;

  node->left = NULL;
  node->right = NULL;
  node->volume_id = -1;
  node->bounds = Box();

  return node;
}

static void free_bvhnode_recursive(VolumeBVHNode *node)
{
  if (node == NULL)
    return;

  free_bvhnode_recursive(node->left);
  free_bvhnode_recursive(node->right);

  delete node;
}

static int is_bvh_leaf(const VolumeBVHNode *node)
{
  return (
    node->left == NULL &&
    node->right == NULL &&
    node->volume_id != -1);
}

static int find_median(VolumePrimitive **volumes, int begin, int end, int axis)
{
  int low, high, mid;
  double key = 0;

  assert(axis >= 0 && axis <= 2);

  low = begin;
  high = end - 1;
  mid = -1;

  switch (axis) {
  case 0: key = (volumes[low]->centroid.x + volumes[high]->centroid.x) / 2; break;
  case 1: key = (volumes[low]->centroid.y + volumes[high]->centroid.y) / 2; break;
  case 2: key = (volumes[low]->centroid.z + volumes[high]->centroid.z) / 2; break;
  default: break;
  }

  while (low != mid) {
    double value = 0;
    mid = (low + high) / 2;

    switch (axis) {
    case 0: value = volumes[mid]->centroid.x; break;
    case 1: value = volumes[mid]->centroid.y; break;
    case 2: value = volumes[mid]->centroid.z; break;
    default: break;
    }

    if (key < value)
      high = mid;
    else if (value < key)
      low = mid;
    else
      break;
  }

  return mid + 1;
}

static int compare_double(double x, double y)
{
  if (x > y)
    return 1;
  else if (x < y)
    return -1;
  else
    return 0;
}

static int volume_compare_x(const void *a, const void *b)
{
  VolumePrimitive **A = (VolumePrimitive **) a;
  VolumePrimitive **B = (VolumePrimitive **) b;
  return compare_double((*A)->centroid.x, (*B)->centroid.x);
}

static int volume_compare_y(const void *a, const void *b)
{
  VolumePrimitive **A = (VolumePrimitive **) a;
  VolumePrimitive **B = (VolumePrimitive **) b;
  return compare_double((*A)->centroid.y, (*B)->centroid.y);
}

static int volume_compare_z(const void *a, const void *b)
{
  VolumePrimitive **A = (VolumePrimitive **) a;
  VolumePrimitive **B = (VolumePrimitive **) b;
  return compare_double((*A)->centroid.z, (*B)->centroid.z);
}

static int ray_volume_intersect (const VolumeAccelerator *acc, int volume_id,
  double time, const Ray *ray, IntervalList *intervals)
{
  Interval interval;
  int hit;

  hit = acc->VolumeIntersect_(acc->volume_set_, volume_id, time, ray, &interval);
  if (!hit) {
    return 0;
  }

  if (interval.tmax < ray->tmin || ray->tmax < interval.tmin) {
    return 0;
  }

  IntervalListPush(intervals, &interval);
  return 1;
}

#if 0
static void swap_isect_ptr(IntervalList **isect0, IntervalList **isect1)
{
  IntervalList *isect_swp = *isect0;
  *isect0 = *isect1;
  *isect1 = isect_swp;
}

static int is_empty(const VolumeBVHNodeStack *stack)
{
  return (stack->depth == 0);
}

static void push_node(VolumeBVHNodeStack *stack, const VolumeBVHNode *node)
{
  stack->node[stack->depth++] = node;
  assert(stack->depth < BVH_STACKSIZE);
}

static const VolumeBVHNode *pop_node(VolumeBVHNodeStack *stack)
{
  assert(!is_empty(stack));
  return stack->node[--stack->depth];
}
#endif

} // namespace xxx
