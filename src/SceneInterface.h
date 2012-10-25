/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef SCENEINTERFACE_H
#define SCENEINTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long int ID;
typedef int Status;
enum { SI_BADID = -1 };
enum { SI_FAIL = -1, SI_SUCCESS = 0 };

/* TODO should be combined with Status? */
enum SiErrorNo {
	SI_ERR_NONE = 0,
	SI_ERR_BADTYPE,
	SI_ERR_FAILOPENPLG,
	SI_ERR_FAILLOAD,
	SI_ERR_FAILNEW,
	/* plugin */
	SI_ERR_PLUGIN_NOT_FOUND,
	SI_ERR_INIT_PLUGIN_FUNC_NOT_EXIST,
	SI_ERR_INIT_PLUGIN_FUNC_FAIL,
	SI_ERR_BAD_PLUGIN_INFO,
	SI_ERR_CLOSE_PLUGIN_FAIL,
	/* no memory */
	SI_ERR_NO_MEMORY
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

/* Error interfaces */
extern int SiGetErrorNo(void);

/* Plugin interfaces */
extern Status SiOpenPlugin(const char *filename);

/* Scene interfaces */
extern Status SiOpenScene(void);
extern Status SiCloseScene(void);
extern Status SiRenderScene(ID renderer);
extern Status SiSaveFrameBuffer(ID framebuffer, const char *filename);
extern Status SiRunProcedure(ID procedure);

extern ID SiNewObjectInstance(ID accelerator);
extern ID SiNewFrameBuffer(const char *arg);
extern ID SiNewTurbulence(void);
extern ID SiNewProcedure(const char *plugin_name);
extern ID SiNewRenderer(void);
extern ID SiNewTexture(const char *filename);
extern ID SiNewCamera(const char *arg);
extern ID SiNewShader(const char *plugin_name);
extern ID SiNewVolume(void);
extern ID SiNewCurve(const char *filename);
extern ID SiNewLight(const char *arg);
extern ID SiNewMesh(const char *filename);

extern Status SiAssignFrameBuffer(ID renderer, ID framebuffer);
extern Status SiAssignTexture(ID shader, const char *prop_name, ID texture);
extern Status SiAssignCamera(ID renderer, ID camera);
extern Status SiAssignShader(ID object, ID shader);

/* Property interfaces */
extern Status SiSetProperty1(ID id, const char *name, double v0);
extern Status SiSetProperty2(ID id, const char *name, double v0, double v1);
extern Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2);
extern Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3);

/* time variable property */
extern Status SiSetSampleProperty3(ID id, const char *name,
		double v0, double v1, double v2, double time);

extern Status SiAssignTurbulence(ID id, const char *name, ID turbulence);
extern Status SiAssignVolume(ID id, const char *name, ID volume);

extern Status SiGetPropertyList(const char *type_name,
		const char ***property_types,
		const char ***property_names,
		int *property_count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

