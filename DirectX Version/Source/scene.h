#ifndef SCENE
#define SCENE

#include "Win32_DX11AppUtil.h"
#include "model.h"

//------------------------------------------------------------------------- 
struct Scene
{
	int     num_models;
	Model * Models[10];

	void    Add(Model * n);

	Scene(int reducedVersion); // Main world

	// Simple latency box (keep similar vertex format and shader params same, for ease of code)
	Scene();

	void Render(Matrix4f view, Matrix4f proj);

};

#endif
