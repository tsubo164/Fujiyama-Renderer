// Copyright (c) 2011-2019 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_scene_node.h"
#include <cassert>

namespace fj {

int SceneNode::SetProperty(const std::string &property_name, const PropertyValue &value)
{
  const Property *property_list = get_property_list();
  const Property *found_property = PropFind(property_list, value.type, property_name.c_str());

  if (found_property == NULL) {
    return -1;
  }

  // TODO is there better way to avoid this pointer?
  return found_property->SetValue(this, value);
}

} // namespace xxx
