/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "SceneInterface.h"
#include "FrameBufferIO.h"
#include "PrimitiveSet.h"
#include "CurveIO.h"
#include "MeshIO.h"
#include "Scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define GET_LAST_ADDED_ID(Type) (ScnGet##Type##Count(scene) - 1)

enum { TYPE_ID_OFFSET = 10000000 };

enum {
	SI_NOERR = 0,
	SI_ERR_BADTYPE,
	SI_ERR_FAILOPENPLG,
	SI_ERR_FAILLOAD,
	SI_ERR_FAILNEW,
	SI_ERR_NOMEM
};

enum {
	Type_Begin = 0,
	Type_ObjectInstance = 1,
	Type_Accelerator,
	Type_FrameBuffer,
	Type_Procedure,
	Type_Renderer,
	Type_Texture,
	Type_Camera,
	Type_Plugin,
	Type_Shader,
	Type_Volume,
	Type_Curve,
	Type_Light,
	Type_Mesh,
	Type_End
};

struct Entry {
	int type;
	int index;
};

static struct Scene *scene = NULL;
static int si_errno = SI_NOERR;

static int is_valid_type(int type);
static ID encode_id(int type, int index);
static struct Entry decode_id(ID id);
static int create_implicit_groups(void);
static void set_errno(int err_no);

/* Property interfaces */
static int SetObjectInstanceProperty1(int index, const char *name, double v0);
static int SetObjectInstanceProperty3(int index, const char *name, double v0, double v1, double v2);

static int SetRendererProperty1(int index, const char *name, double v0);
static int SetRendererProperty2(int index, const char *name, double v0, double v1);
static int SetRendererProperty4(int index, const char *name, double v0, double v1, double v2, double v3);

static int SetCameraProperty1(int index, const char *name, double v0);
static int SetCameraProperty3(int index, const char *name, double v0, double v1, double v2);

static int SetShaderProperty1(int index, const char *name, double v0);
static int SetShaderProperty2(int index, const char *name, double v0, double v1);
static int SetShaderProperty3(int index, const char *name, double v0, double v1, double v2);

static int SetLightProperty1(int index, const char *name, double v0);
static int SetLightProperty3(int index, const char *name, double v0, double v1, double v2);

/* Error interfaces */
int SiGetErrorNo(void)
{
	return si_errno;
}

const char *SiGetErrorMessage(int err_no)
{
	static const char *errmsg[] = {
		"",                   /* SI_NOERR */
		"bad arg",            /* SI_ERR_BADARG */
		"invalid entry type", /* SI_ERR_BADTYPE */
		"open plugin failed", /* SI_ERR_FAILOPENPLG */
		"load file failed",   /* SI_ERR_FAILLOAD */
		"new entry failed",   /* SI_ERR_FAILNEW */
		"no memory"           /* SI_ERR_NOMEM */
	};
	static const int nerrs = (int) sizeof(errmsg)/sizeof(errmsg[0]);

	if (err_no >= nerrs) {
		fprintf(stderr, "Logic error: err_no %d is out of range\n", err_no);
		abort();
	}
	return errmsg[err_no];
}

/* Plugin interfaces */
Status SiOpenPlugin(const char *filename)
{
	if (ScnOpenPlugin(scene, filename) == NULL) {
		set_errno(SI_ERR_FAILOPENPLG);
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

/* Scene interfaces */
Status SiOpenScene(void)
{
	scene = ScnNew();

	if (scene == NULL) {
		set_errno(SI_ERR_NOMEM);
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

Status SiCloseScene(void)
{
	ScnFree(scene);
    scene = NULL;

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

Status SiRenderScene(ID renderer)
{
	int err = 0;

	err = create_implicit_groups();
	if (err) {
		/* TODO error handling */
		return SI_FAIL;
	}

	/* TODO fix hard-coded renderer index */
	err = RdrRender(ScnGetRenderer(scene, 0));
	if (err) {
		/* TODO error handling */
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

Status SiSaveFrameBuffer(ID framebuffer, const char *filename)
{
	const struct Entry entry = decode_id(framebuffer);
	struct FrameBuffer *framebuffer_ptr = NULL;
	int err = 0;

	if (entry.type != Type_FrameBuffer)
		return SI_FAIL;

	framebuffer_ptr = ScnGetFrameBuffer(scene, entry.index);
	if (framebuffer_ptr == NULL)
		return SI_FAIL;

	err = FbSaveCroppedData(framebuffer_ptr, filename);
	if (err) {
		/* TODO error handling */
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

Status SiRunProcedure(ID procedure)
{
	const struct Entry entry = decode_id(procedure);
	struct Procedure *procedure_ptr = NULL;
	int err = 0;

	if (entry.type != Type_Procedure)
		return SI_FAIL;

	procedure_ptr = ScnGetProcedure(scene, entry.index);
	if (procedure_ptr == NULL)
		return SI_FAIL;

	err = PrcRun(procedure_ptr);
	if (err) {
		/* TODO error handling */
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return SI_SUCCESS;
}

/* TODO change argument name accelerator */
ID SiNewObjectInstance(ID accelerator)
{
	const struct Entry entry = decode_id(accelerator);

	if (entry.type == Type_Accelerator) {
		struct Accelerator *acc = NULL;

		acc = ScnGetAccelerator(scene, entry.index);
		if (acc == NULL) {
			set_errno(SI_ERR_BADTYPE);
			return SI_BADID;
		}

		/* TODO come up with another way to pass acc */
		if (ScnNewObjectInstance(scene, acc) == NULL) {
			set_errno(SI_ERR_FAILNEW);
			return SI_BADID;
		}
	}
	else if (entry.type == Type_Volume) {
		struct ObjectInstance *object = NULL;
		struct Volume *volume = NULL;
		int err = 0;

		volume = ScnGetVolume(scene, entry.index);
		if (volume == NULL) {
			set_errno(SI_ERR_BADTYPE);
			return SI_BADID;
		}

		object = ScnNewObjectInstance(scene, NULL);
		if (object == NULL) {
			set_errno(SI_ERR_FAILNEW);
			return SI_BADID;
		}

		err = ObjSetVolume(object, volume);
		if (err) {
			set_errno(SI_ERR_FAILNEW);
			return SI_BADID;
		}

	}
	else {
		set_errno(SI_ERR_BADTYPE);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_ObjectInstance, GET_LAST_ADDED_ID(ObjectInstance));
}

ID SiNewFrameBuffer(const char *arg)
{
	if (ScnNewFrameBuffer(scene) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_FrameBuffer, GET_LAST_ADDED_ID(FrameBuffer));
}

ID SiNewProcedure(const char *plugin_name)
{
	struct Plugin **plugins = ScnGetPluginList(scene);
	struct Plugin *found = NULL;
	const int N = (int) ScnGetPluginCount(scene);
	int i = 0;

	for (i = 0; i < N; i++) {
		if (strcmp(plugin_name, PlgGetName(plugins[i])) == 0) {
			found = plugins[i];
			break;
		}
	}
	if (found == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}
	if (ScnNewProcedure(scene, found) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Procedure, GET_LAST_ADDED_ID(Procedure));
}

ID SiNewRenderer(void)
{
	if (ScnNewRenderer(scene) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Renderer, GET_LAST_ADDED_ID(Renderer));
}

ID SiNewTexture(const char *filename)
{
	struct Texture *tex = NULL;

	tex = ScnNewTexture(scene);
	if (tex == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}
	if (TexLoadFile(tex, filename)) {
		set_errno(SI_ERR_FAILLOAD);
		return SI_FAIL;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Texture, GET_LAST_ADDED_ID(Texture));
}

ID SiNewCamera(const char *arg)
{
	if (ScnNewCamera(scene, arg) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Camera, GET_LAST_ADDED_ID(Camera));
}

ID SiNewShader(const char *plugin_name)
{
	struct Plugin **plugins = ScnGetPluginList(scene);
	struct Plugin *found = NULL;
	const int N = (int) ScnGetPluginCount(scene);
	int i = 0;

	for (i = 0; i < N; i++) {
		if (strcmp(plugin_name, PlgGetName(plugins[i])) == 0) {
			found = plugins[i];
			break;
		}
	}
	if (found == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}
	if (ScnNewShader(scene, found) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Shader, GET_LAST_ADDED_ID(Shader));
}

ID SiNewVolume(void)
{
	struct Volume *volume = NULL;

	volume = ScnNewVolume(scene);
	if (volume == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Volume, GET_LAST_ADDED_ID(Volume));
}

ID SiNewCurve(const char *filename)
{
	struct Curve *curve = NULL;
	struct Accelerator *acc = NULL;
	struct PrimitiveSet primset;

	curve = ScnNewCurve(scene);
	if (curve == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}
	if (CrvLoadFile(curve, filename)) {
		set_errno(SI_ERR_FAILLOAD);
		return SI_BADID;
	}

	acc = ScnNewAccelerator(scene, ACC_GRID);
	if (acc == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	CrvGetPrimitiveSet(curve, &primset);
	AccSetPrimitiveSet(acc, &primset);

	set_errno(SI_NOERR);
	return encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
}

ID SiNewLight(const char *arg)
{
	if (ScnNewLight(scene, arg) == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	set_errno(SI_NOERR);
	return encode_id(Type_Light, GET_LAST_ADDED_ID(Light));
}

ID SiNewMesh(const char *filename)
{
	struct Mesh *mesh = NULL;
	struct Accelerator *acc = NULL;
	struct PrimitiveSet primset;

	mesh = ScnNewMesh(scene);
	if (mesh == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}
	if (MshLoadFile(mesh, filename)) {
		set_errno(SI_ERR_FAILLOAD);
		return SI_BADID;
	}

	acc = ScnNewAccelerator(scene, ACC_GRID);
	if (acc == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_BADID;
	}

	MshGetPrimitiveSet(mesh, &primset);
	AccSetPrimitiveSet(acc, &primset);

	set_errno(SI_NOERR);
	return encode_id(Type_Accelerator, GET_LAST_ADDED_ID(Accelerator));
}

Status SiAssignShader(ID object, ID shader)
{
	struct ObjectInstance *object_ptr = NULL;
	struct Shader *shader_ptr = NULL;
	{
		const struct Entry entry = decode_id(object);

		if (entry.type != Type_ObjectInstance)
			return SI_FAIL;

		object_ptr = ScnGetObjectInstance(scene, entry.index);
		if (object_ptr == NULL)
			return SI_FAIL;
	}
	{
		const struct Entry entry = decode_id(shader);

		if (entry.type != Type_Shader)
			return SI_FAIL;

		shader_ptr = ScnGetShader(scene, entry.index);
		if (shader_ptr == NULL)
			return SI_FAIL;
	}

	ObjSetShader(object_ptr, shader_ptr);
	return SI_SUCCESS;
}

Status SiAssignTexture(ID shader, const char *prop_name, ID texture)
{
	struct PropertyValue value = InitPropValue();
	struct Shader *shader_ptr = NULL;
	struct Texture *texture_ptr = NULL;
	{
		const struct Entry entry = decode_id(shader);

		if (entry.type != Type_Shader)
			return SI_FAIL;

		shader_ptr = ScnGetShader(scene, entry.index);
		if (shader_ptr == NULL)
			return SI_FAIL;
	}
	{
		const struct Entry entry = decode_id(texture);

		if (entry.type != Type_Texture)
			return SI_FAIL;

		texture_ptr = ScnGetTexture(scene, entry.index);
		if (texture_ptr == NULL)
			return SI_FAIL;
	}

	value = PropTexture(texture_ptr);
	return ShdSetProperty(shader_ptr, prop_name, &value);
}

Status SiAssignCamera(ID renderer, ID camera)
{
	struct Renderer *renderer_ptr = NULL;
	struct Camera *camera_ptr = NULL;
	{
		const struct Entry entry = decode_id(renderer);

		if (entry.type != Type_Renderer)
			return SI_FAIL;

		renderer_ptr = ScnGetRenderer(scene, entry.index);
		if (renderer_ptr == NULL)
			return SI_FAIL;
	}
	{
		const struct Entry entry = decode_id(camera);

		if (entry.type != Type_Camera)
			return SI_FAIL;

		camera_ptr = ScnGetCamera(scene, entry.index);
		if (camera_ptr == NULL)
			return SI_FAIL;
	}

	RdrSetCamera(renderer_ptr, camera_ptr);
	return SI_SUCCESS;
}

Status SiAssignFrameBuffer(ID renderer, ID framebuffer)
{
	struct Renderer *renderer_ptr = NULL;
	struct FrameBuffer *framebuffer_ptr = NULL;
	{
		const struct Entry entry = decode_id(renderer);

		if (entry.type != Type_Renderer)
			return SI_FAIL;

		renderer_ptr = ScnGetRenderer(scene, entry.index);
		if (renderer_ptr == NULL)
			return SI_FAIL;
	}
	{
		const struct Entry entry = decode_id(framebuffer);

		if (entry.type != Type_FrameBuffer)
			return SI_FAIL;

		framebuffer_ptr = ScnGetFrameBuffer(scene, entry.index);
		if (framebuffer_ptr == NULL)
			return SI_FAIL;
	}

	RdrSetFrameBuffers(renderer_ptr, framebuffer_ptr);
	return SI_SUCCESS;
}


Status SiSetProperty1(ID id, const char *name, double v0)
{
	const struct Entry entry = decode_id(id);
	int err = 0;

	switch (entry.type) {
	case Type_ObjectInstance:
		err = SetObjectInstanceProperty1(entry.index, name, v0);
		break;
	case Type_Renderer:
		err = SetRendererProperty1(entry.index, name, v0);
		break;
	case Type_Camera:
		err = SetCameraProperty1(entry.index, name, v0);
		break;
	case Type_Shader:
		err = SetShaderProperty1(entry.index, name, v0);
		break;
	case Type_Light:
		err = SetLightProperty1(entry.index, name, v0);
		break;
	default:
		assert(!is_valid_type(entry.type) && "Some types are not implemented yet");
		break;
	}

	if (err)
		return SI_FAIL;
	else
		return SI_SUCCESS;
}

Status SiSetProperty2(ID id, const char *name, double v0, double v1)
{
	const struct Entry entry = decode_id(id);
	int err = 0;

	switch (entry.type) {
	case Type_Renderer:
		err = SetRendererProperty2(entry.index, name, v0, v1);
		break;
	case Type_Shader:
		err = SetShaderProperty2(entry.index, name, v0, v1);
		break;
	default:
		assert(!is_valid_type(entry.type) && "Some types are not implemented yet");
		break;
	}

	if (err)
		return SI_FAIL;
	else
		return SI_SUCCESS;
}

Status SiSetProperty3(ID id, const char *name, double v0, double v1, double v2)
{
	const struct Entry entry = decode_id(id);
	int err = 0;

	switch (entry.type) {
	case Type_ObjectInstance:
		err = SetObjectInstanceProperty3(entry.index, name, v0, v1, v2);
		break;
	case Type_Camera:
		err = SetCameraProperty3(entry.index, name, v0, v1, v2);
		break;
	case Type_Shader:
		/* TODO NEXT check return value */
		err = SetShaderProperty3(entry.index, name, v0, v1, v2);
		break;
	case Type_Light:
		err = SetLightProperty3(entry.index, name, v0, v1, v2);
		break;
	default:
		assert(!is_valid_type(entry.type) && "Some types are not implemented yet");
		break;
	}

	if (err)
		return SI_FAIL;
	else
		return SI_SUCCESS;
}

Status SiSetProperty4(ID id, const char *name, double v0, double v1, double v2, double v3)
{
	const struct Entry entry = decode_id(id);
	int err = 0;

	switch (entry.type) {
	case Type_Renderer:
		err = SetRendererProperty4(entry.index, name, v0, v1, v2, v3);
		break;
	default:
		assert(!is_valid_type(entry.type) && "Some types are not implemented yet");
		break;
	}

	if (err)
		return SI_FAIL;
	else
		return SI_SUCCESS;
}

Status SiAssignVolume(ID id, const char *name, ID volume)
{
	const struct Entry entry = decode_id(id);
	const struct Entry volume_ent = decode_id(volume);
	struct PropertyValue value = InitPropValue();
	struct Volume *volume_ptr = NULL;
	int err = 0;

	if (volume_ent.type != Type_Volume)
		return SI_FAIL;

	volume_ptr = ScnGetVolume(scene, volume_ent.index);
	if (volume_ptr == NULL)
		return SI_FAIL;

	value = PropVolume(volume_ptr);

	switch (entry.type) {
	case Type_Procedure:
		{
			struct Procedure *procedure_ptr = ScnGetProcedure(scene, entry.index);
			err = PrcSetProperty(procedure_ptr, name, &value);
		}
		break;
	default:
		assert(!is_valid_type(entry.type) && "Some types are not implemented yet");
		break;
	}

	if (err)
		return SI_FAIL;

	return SI_SUCCESS;
}

static int is_valid_type(int type)
{
	return type > Type_Begin && type < Type_End;
}

static ID encode_id(int type, int index)
{
	return TYPE_ID_OFFSET * type + index;
}

static struct Entry decode_id(ID id)
{
	struct Entry entry = {-1, -1};
	const int type = id / TYPE_ID_OFFSET;

	if (!is_valid_type(type))
		return entry;

	entry.type = type;
	entry.index = id - (type * TYPE_ID_OFFSET);

	return entry;
}

/* ObjectInstance Property */
static int SetObjectInstanceProperty1(int index, const char *name, double v0)
{
	int result = SI_SUCCESS;
	struct ObjectInstance *obj = ScnGetObjectInstance(scene, index);
	if (obj == NULL)
		return SI_FAIL;

	if (strcmp(name, "transform_order") == 0) {
		ObjSetTransformOrder(obj, (int) v0);
	} else if (strcmp(name, "rotate_order") == 0) {
		ObjSetRotateOrder(obj, (int) v0);
	} else {
		result = SI_FAIL;
	}

	return result;
}

static int SetObjectInstanceProperty3(int index, const char *name, double v0, double v1, double v2)
{
	int result = SI_SUCCESS;
	struct ObjectInstance *obj = ScnGetObjectInstance(scene, index);
	if (obj == NULL)
		return SI_FAIL;

	if (strcmp(name, "translate") == 0) {
		ObjSetTranslate(obj, v0, v1, v2);
	} else if (strcmp(name, "rotate") == 0) {
		ObjSetRotate(obj, v0, v1, v2);
	} else if (strcmp(name, "scale") == 0) {
		ObjSetScale(obj, v0, v1, v2);
	} else {
		result = SI_FAIL;
	}

	return result;
}

/* Renderer Property */
static int SetRendererProperty1(int index, const char *name, double v0)
{
	int result = SI_SUCCESS;
	struct Renderer *renderer = ScnGetRenderer(scene, index);
	if (renderer == NULL)
		return SI_FAIL;

	if (strcmp(name, "cast_shadow") == 0) {
		RdrSetShadowEnable(renderer, (int) v0);
	} else if (strcmp(name, "max_reflect_depth") == 0) {
		RdrSetMaxReflectDepth(renderer, (int) v0);
	} else if (strcmp(name, "max_refract_depth") == 0) {
		RdrSetMaxRefractDepth(renderer, (int) v0);
	} else if (strcmp(name, "raymarch_step") == 0) {
		RdrSetRaymarchStep(renderer, v0);
	} else if (strcmp(name, "raymarch_shadow_step") == 0) {
		RdrSetRaymarchShadowStep(renderer, v0);
	} else if (strcmp(name, "raymarch_reflect_step") == 0) {
		RdrSetRaymarchReflectStep(renderer, v0);
	} else if (strcmp(name, "raymarch_refract_step") == 0) {
		RdrSetRaymarchRefractStep(renderer, v0);
	} else {
		result = SI_FAIL;
	}

	return result;
}

static int SetRendererProperty2(int index, const char *name, double v0, double v1)
{
	int result = SI_SUCCESS;
	struct Renderer *renderer = ScnGetRenderer(scene, index);
	if (renderer == NULL)
		return SI_FAIL;

	if (strcmp(name, "resolution") == 0) {
		RdrSetResolution(renderer, (int) v0, (int) v1);
	} else if (strcmp(name, "pixelsamples") == 0) {
		RdrSetPixelSamples(renderer, (int) v0, (int) v1);
	} else if (strcmp(name, "tilesize") == 0) {
		RdrSetTileSize(renderer, (int) v0, (int) v1);
	} else if (strcmp(name, "filterwidth") == 0) {
		RdrSetFilterWidth(renderer, v0, v1);
	} else {
		result = SI_FAIL;
	}

	return result;
}

static int SetRendererProperty4(int index, const char *name, double v0, double v1, double v2, double v3)
{
	int result = SI_SUCCESS;
	struct Renderer *renderer = ScnGetRenderer(scene, index);
	if (renderer == NULL)
		return SI_FAIL;

	if (strcmp(name, "render_region") == 0) {
		RdrSetRenderRegion(renderer, (int) v0, (int) v1, (int) v2, (int) v3);
	} else {
		result = SI_FAIL;
	}

	return result;
}

/* Camera Property */
static int SetCameraProperty1(int index, const char *name, double v0)
{
	int result = SI_SUCCESS;
	struct Camera *cam = ScnGetCamera(scene, index);
	if (cam == NULL)
		return SI_FAIL;

	if (strcmp(name, "fov") == 0) {
		CamSetFov(cam, v0);
	} else if (strcmp(name, "znear") == 0) {
		CamSetNearPlane(cam, v0);
	} else if (strcmp(name, "zfar") == 0) {
		CamSetFarPlane(cam, v0);
	} else {
		result = SI_FAIL;
	}

	return result;
}

static int SetCameraProperty3(int index, const char *name, double v0, double v1, double v2)
{
	int result = SI_SUCCESS;
	struct Camera *cam = ScnGetCamera(scene, index);
	if (cam == NULL)
		return SI_FAIL;

	if (strcmp(name, "position") == 0) {
		CamSetPosition(cam, v0, v1, v2);
	} else if (strcmp(name, "direction") == 0) {
		CamSetDirection(cam, v0, v1, v2);
	} else {
		result = SI_FAIL;
	}

	return result;
}

/* Shader Property */
static int SetShaderProperty1(int index, const char *name, double v0)
{
	const struct PropertyValue value = PropScalar(v0);
	struct Shader *shader = ScnGetShader(scene, index);

	if (shader == NULL)
		return SI_FAIL;

	return ShdSetProperty(shader, name, &value);
}

static int SetShaderProperty2(int index, const char *name, double v0, double v1)
{
	const struct PropertyValue value = PropVector2(v0, v1);
	struct Shader *shader = ScnGetShader(scene, index);

	if (shader == NULL)
		return SI_FAIL;

	return ShdSetProperty(shader, name, &value);
}

static int SetShaderProperty3(int index, const char *name, double v0, double v1, double v2)
{
	const struct PropertyValue value = PropVector3(v0, v1, v2);
	struct Shader *shader = ScnGetShader(scene, index);

	if (shader == NULL)
		return SI_FAIL;

	return ShdSetProperty(shader, name, &value);
}

/* Light Property */
static int SetLightProperty1(int index, const char *name, double v0)
{
	int result = SI_SUCCESS;
	struct Light *light = ScnGetLight(scene, index);
	if (light == NULL)
		return SI_FAIL;

	if (strcmp(name, "intensity") == 0) {
		LgtSetIntensity(light, v0);
	} else {
		result = SI_FAIL;
	}

	return result;
}

static int SetLightProperty3(int index, const char *name, double v0, double v1, double v2)
{
	int result = SI_SUCCESS;
	struct Light *light = ScnGetLight(scene, index);
	if (light == NULL)
		return SI_FAIL;

	if (strcmp(name, "position") == 0) {
		LgtSetPosition(light, v0, v1, v2);
	} else {
		result = SI_FAIL;
	}

	return result;
}

/* Procedure Property */

static int create_implicit_groups(void)
{
	size_t i;
	struct ObjectGroup *all_objects;
	struct Renderer *renderer;

	all_objects = ScnNewObjectGroup(scene);
	if (all_objects == NULL) {
		set_errno(SI_ERR_FAILNEW);
		return SI_FAIL;
	}

	for (i = 0; i < ScnGetObjectInstanceCount(scene); i++) {
		struct ObjectInstance *obj = ScnGetObjectInstance(scene, i);
		ObjGroupAdd(all_objects, obj);
	}

	/* Preparing ObjectInstance */
	for (i = 0; i < ScnGetObjectInstanceCount(scene); i++) {
		struct ObjectInstance *obj = ScnGetObjectInstance(scene, i);
		const struct Light **lightlist = (const struct Light **) ScnGetLightList(scene);
		int nlights = ScnGetLightCount(scene);
		ObjSetLightList(obj, lightlist, nlights);

		if (ObjGetReflectTarget(obj) == NULL)
			ObjSetReflectTarget(obj, all_objects);

		if (ObjGetRefractTarget(obj) == NULL)
			ObjSetRefractTarget(obj, all_objects);
	}

	renderer = ScnGetRenderer(scene, 0);
	RdrSetTargetObjects(renderer, all_objects);

	return SI_SUCCESS;
}

static void set_errno(int err_no)
{
	si_errno = err_no;
}

