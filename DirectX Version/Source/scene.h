#ifndef SCENE
#define SCENE

#include "Win32_DX11AppUtil.h"
#include "model.h"
#include "DDSTextureLoader\DDSTextureLoader.h"
#include "DirectXTex\DirectXTex.h"

using namespace DirectX;


//------------------------------------------------------------------------- 
struct Scene
{
	
	// used when shaderfilling objects, allows for eventually creating more objects
	enum modelTypes
	{
		Box, Skybox
	};

	int     num_models;
	Model * Models[10];
	// used to change textures
	ShaderFill * generated_texture[5];
	Vector3f monitorOffset = Vector3f(1.2,0,0);//used by getters and setters for adding monitors
	//float monitorHeight;
	//startFloat startingPoint;
	float monitorHeight = 1;
	float monitorWidth = 1;

	void  Add(Model * n);
	Vector3f getLastMonitorPosition();//return the positon of the last monitor
	Vector3f getOffset();//sets position of new monitor based on offset //TODO
	void setOffset(Vector3f);//sets monitor offset
	void addMonitor();//adds monitor to scene uses position of last monitor
	Scene(int reducedVersion); // Main world

	// Simple latency box (keep similar vertex format and shader params same, for ease of code)
	Scene();

	void Render(Matrix4f view, Matrix4f proj);


};

//can probably get rid of
struct startFloat {
	float x1;
	float y1;
	float z1;
	float x2;
	float y2;
	float z2;
	Model::Color color;
	startFloat(float _x1, float _y1, float _z1, float _x2, float _y2, float _z2, Model::Color c) : x1(_x1),
		y1(_y1), z1(_z1), x2(_x2), y2(_y2), z2(_z2), color(c){}
};

#endif
