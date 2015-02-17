#ifndef MODEL
#define MODEL

#include "Kernel/OVR_Math.h"
#include "Win32_DX11AppUtil.h"
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

	Vector3f     Pos;
	Vector3f	 OriginalPos;
	Quatf        Rot;
	Quatf		 OriginalRot;
	Matrix4f     Mat;
	int          numVertices, numIndices;
	Vertex       Vertices[2000]; //Note fixed maximum
	uint16_t     Indices[2000];
	ShaderFill * Fill;
	DataBuffer * VertexBuffer, *IndexBuffer;
	int size;

	Model(Vector3f arg_pos, ShaderFill * arg_Fill);
	Matrix4f& GetMatrix();
	
	void setOriginalPos();

	void AddVertex(const Vertex& v);
	
	void AddIndex(uint16_t a);

	void AllocateBuffers();

	void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, Color c);
};

#endif
