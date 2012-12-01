/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

/*
example scene of C interfaces
1 cube with 1 point lights

to compile and render this scene, run this at the top level of source tree

 $ make sample

*/

#include "SceneInterface.h"
#include <stdio.h>

int main(int argc, const char **argv)
{
	const int W = 320;
	const int H = 240;

	ID framebuffer;
	ID renderer;
	ID camera;
	ID object;
	ID shader;
	ID light;
	ID mesh;

	/* Scene */
	SiOpenScene();

	/* Plugin */
	if (SiOpenPlugin("PlasticShader.so")) {
		/* TODO error handling */
		/*
		fprintf(stderr, "Could not open shader: %s\n", SiGetErrorMessage(SiGetErrorNo()));
		*/
	}

	/* Camera */
	camera = SiNewCamera("PerspectiveCamera");
	if (camera == SI_BADID) {
		fprintf(stderr, "Could not allocate camera\n");
		return -1;
	}
	SiSetProperty3(camera, "translate", 3, 3, 3);
	SiSetProperty3(camera, "rotate", -35.264389682754654, 45, 0);

	/* Light */
	light = SiNewLight(SI_POINT_LIGHT);
	if (light  == SI_BADID) {
		fprintf(stderr, "Could not allocate light\n");
		return -1;
	}
	SiSetProperty3(light, "translate", 1, 12, 3);

	/* Shader */
	shader = SiNewShader("PlasticShader");
	if (shader == SI_BADID) {
		fprintf(stderr, "Could not create shader: PlasticShader\n");
		return -1;
	}

	/* Mesh and Accelerator */
	mesh = SiNewMesh("scenes/cube.mesh");
	if (mesh == SI_BADID) {
		/* TODO error handling */
		/*
		fprintf(stderr, "Could not create mesh: %s\n", SiGetErrorMessage(SiGetErrorNo()));
		*/
		return -1;
	}

	/* ObjectInstance */
	object = SiNewObjectInstance(mesh);
	if (object == SI_BADID) {
		fprintf(stderr, "Could not create object instance\n");
		return -1;
	}
	SiSetProperty3(object, "rotate", 0, 10, 0);
	SiAssignShader(object, shader);

	/* FrameBuffer */
	framebuffer = SiNewFrameBuffer("rgba");
	if (framebuffer == SI_BADID) {
		fprintf(stderr, "Could not allocate framebuffer\n");
		return -1;
	}

	/* Renderer */
	renderer = SiNewRenderer();
	if (renderer == SI_BADID) {
		fprintf(stderr, "Could not allocate renderer\n");
		return -1;
	}
	SiSetProperty2(renderer, "resolution", W, H);
	SiAssignCamera(renderer, camera);
	SiAssignFrameBuffer(renderer, framebuffer);

	/* Render scene */
	SiRenderScene(renderer);
	SiSaveFrameBuffer(framebuffer, "scenes/cube.fb");
	SiCloseScene();

	return 0;
}

