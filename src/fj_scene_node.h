// Copyright (c) 2011-2020 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SCENE_NODE_H
#define FJ_SCENE_NODE_H

#include "fj_property.h"
#include <string>

namespace fj {

class FJ_API SceneNode {
public:
  SceneNode() {}
  virtual ~SceneNode() {}

  int SetProperty(const std::string &property_name, const PropertyValue &value);

private:
  virtual const Property *get_property_list() const = 0;
};

} // namespace xxx

#endif // FJ_XXX_H
