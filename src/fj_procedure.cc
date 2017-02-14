// Copyright (c) 2011-2016 Hiroshi Tsubokawa
// See LICENSE and README

#include "fj_procedure.h"
#include "fj_timer.h"

#include <cstdio>
#include <cassert>

namespace fj {

//static int error_no = PRC_ERR_NOERR;
//static void set_error(int err);

Procedure::Procedure()
{
}

Procedure::~Procedure()
{
}

int Procedure::Run() const
{
  // TODO come up with the best place to put message
  printf("Running Procedure ...\n");

  Timer timer;
  timer.Start();

  const int err = run();

  const Elapse elapse = timer.GetElapse();

  if (err) {
    printf("Error: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return -1;
  } else {
    printf("Done: %dh %dm %ds\n", elapse.hour, elapse.min, elapse.sec);
    return 0;
  }
}

int Procedure::SetProperty(const std::string &prop_name, const PropertyValue &src_data)
{
  const Property *shd_props = get_property_list();
  const Property *dst_prop  = PropFind(shd_props, src_data.type, prop_name.c_str());

  if (dst_prop == NULL) {
    return -1;
  }

  assert(dst_prop->SetProperty != NULL);
  // TODO is there better way to avoid this pointer?
  return dst_prop->SetProperty(this, &src_data);
}

/*
static void set_error(int err)
{
  error_no = err;
}
*/

} // namespace xxx
