// Copyright (c) 2011-2017 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef FJ_SCENEINTERFACE_H
#define FJ_SCENEINTERFACE_H

#include "fj_compatibility.h"
#include "fj_callback.h"
#include "fj_renderer.h"

namespace fj {

typedef long int ID;
typedef int Status;
enum { SI_BADID = -1 };
enum { SI_FAIL = -1, SI_SUCCESS = 0 };

enum SiErrorNo {
  SI_ERR_NONE = 0,
  SI_ERR_NO_MEMORY,
  SI_ERR_BADTYPE,
  SI_ERR_FAILLOAD,
  SI_ERR_FAILNEW,
  /* plugin */
  SI_ERR_PLUGIN_NOT_FOUND,
  SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST,
  SI_ERR_INIT_PLUGIN_FUNC_FAIL,
  SI_ERR_BAD_PLUGIN_INFO,
  SI_ERR_CLOSE_PLUGIN_FAIL,
  /* undefined */
  SI_ERR_UNDEFINED
};

enum SiTransformOrder {
  /* transform orders */
  SI_ORDER_SRT = 0,
  SI_ORDER_STR,
  SI_ORDER_RST,
  SI_ORDER_RTS,
  SI_ORDER_TRS,
  SI_ORDER_TSR,
  /* rotate orders */
  SI_ORDER_XYZ,
  SI_ORDER_XZY,
  SI_ORDER_YXZ,
  SI_ORDER_YZX,
  SI_ORDER_ZXY,
  SI_ORDER_ZYX
};

enum SiLightType {
  SI_POINT_LIGHT = 0,
  SI_GRID_LIGHT,
  SI_SPHERE_LIGHT,
  SI_DOME_LIGHT
};

enum SiSamplerType {
  SI_FIXED_GRID_SAMPLER = RENDERER_FIXED_GRID_SAMPLER,
  SI_ADAPTIVE_GRID_SAMPLER = RENDERER_ADAPTIVE_GRID_SAMPLER
};

/* Error interfaces */
FJ_API int SiGetErrorNo(void);

/* Plugin interfaces */
FJ_API ID SiOpenPlugin(const char *filename);

/* Scene interfaces */
FJ_API Status SiOpenScene(void);
FJ_API Status SiCloseScene(void);
FJ_API Status SiRenderScene(ID renderer);
FJ_API Status SiSaveFrameBuffer(ID framebuffer, const char *filename);
FJ_API Status SiRunProcedure(ID procedure);

FJ_API Status SiAddObjectToGroup(ID group, ID object);

FJ_API ID SiNewObjectInstance(ID primset);
FJ_API ID SiNewFrameBuffer(const char *arg);
FJ_API ID SiNewObjectGroup(void);
FJ_API ID SiNewPointCloud(const char *filename);
FJ_API ID SiNewTurbulence(void);
FJ_API ID SiNewProcedure(ID plugin);
FJ_API ID SiNewRenderer(void);
FJ_API ID SiNewTexture(const char *filename);
FJ_API ID SiNewCamera(const char *arg);
FJ_API ID SiNewShader(ID plugin);
FJ_API ID SiNewVolume(void);
FJ_API ID SiNewCurve(const char *filename);
FJ_API ID SiNewLight(int light_type);
FJ_API ID SiNewMesh(const char *filename);

FJ_API Status SiAssignFrameBuffer(ID renderer, ID framebuffer);
FJ_API Status SiAssignObjectGroup(ID id, const char *name, ID group);
FJ_API Status SiAssignPointCloud(ID id, const char *name, ID pointcloud);
FJ_API Status SiAssignTurbulence(ID id, const char *name, ID turbulence);
FJ_API Status SiAssignTexture(ID id, const char *name, ID texture);
FJ_API Status SiAssignVolume(ID id, const char *name, ID volume);
FJ_API Status SiAssignCamera(ID renderer, ID camera);
FJ_API Status SiAssignShader(ID object, const char *shading_group, ID shader);
FJ_API Status SiAssignMesh(ID id, const char *name, ID mesh);

/* Property interfaces */
FJ_API Status SiSetProperty1(ID id, const char *name, double v0);
FJ_API Status SiSetProperty2(ID id, const char *name, double v0, double v1);
FJ_API Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2);
FJ_API Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3);
FJ_API Status SiSetStringProperty(ID id, const char *name, const char *string);

/* time variable property */
FJ_API Status SiSetSampleProperty3(ID id, const char *name,
    double v0, double v1, double v2, double time);

class Property;
FJ_API const Property *SiGetPropertyList(const char *type_name);

/* Callback interfaces */
FJ_API Status SiSetFrameReportCallback(ID id, void *data,
    FrameStartCallback frame_start,
    FrameAbortCallback frame_abort,
    FrameDoneCallback frame_done);

FJ_API Status SiSetTileReportCallback(ID id, void *data,
    TileStartCallback tile_start,
    SampleDoneCallback sample_done,
    TileDoneCallback tile_done);

} // namespace xxx

#endif // FJ_XXX_H
