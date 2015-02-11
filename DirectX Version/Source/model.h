#ifndef MODEL
#define MODEL

#include "Kernel/OVR_Math.h"
#include "Win32_DX11AppUtil.h"
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>

using namespace DirectX;
using namespace std;

//---------------------------------------------------------------------------
struct Model
{
	struct Color
	{
		unsigned char R, G, B, A;

		Color(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 0xff)
			: R(r), G(g), B(b), A(a)
		{ }
	};

	struct Vertex
	{
		Vector3f  Pos;
		Color     C;
		float     U, V;
	};

	/*
	struct SkyboxVertex	//Overloaded SkyboxVertex Structure
	{
		SkyboxVertex(){}
		SkyboxVertex(float x, float y, float z,
			float u, float v,
			float nx, float ny, float nz)
			: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz){}

		XMFLOAT3 pos;
		XMFLOAT2 texCoord;
		XMFLOAT3 normal;
	};

	int NumSphereVertices;
	int NumSphereFaces;

	XMMATRIX Rotationx;
	XMMATRIX Rotationy;
	XMMATRIX Rotationz;
	*/

	Vector3f     Pos;
	Quatf        Rot;
	Matrix4f     Mat;
	int          numVertices, numIndices;
	Vertex       Vertices[2000]; //Note fixed maximum
	uint16_t     Indices[2000];
	ShaderFill * Fill;
	DataBuffer * VertexBuffer, *IndexBuffer;

	Model(Vector3f arg_pos, ShaderFill * arg_Fill);
	Matrix4f& GetMatrix();
	void AddVertex(const Vertex& v);
	void AddIndex(uint16_t a);

	void AllocateBuffers();

	void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, Color c);

	//void CreateSphere(int LatLines, int LongLines);
};

#endif
