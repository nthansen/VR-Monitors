#include "model.h"

Model::Model(Vector3f arg_pos, ShaderFill * arg_Fill) 
{ 
	numVertices = 0; numIndices = 0; 
	Pos = arg_pos; 
	OriginalPos = Pos;
	OriginalRot = Quatf();
	Fill = arg_Fill; 
	size = 1;
}

Matrix4f& Model::GetMatrix()	
{ 
	Mat = Matrix4f(Rot); Mat = Matrix4f::Translation(Pos) * Mat; 
	return Mat; 
}

void Model::AddVertex(const Vertex& v)               
{ 
	Vertices[numVertices++] = v; 
	OVR_ASSERT(numVertices<2000); 
}

void Model::AddIndex(uint16_t a)
{ 
	Indices[numIndices++] = a;   
	OVR_ASSERT(numIndices<2000); 
}

void Model::setOriginalPos() {
	Pos = OriginalPos;
	Rot = OriginalRot;
}

void Model::AllocateBuffers()
{
	VertexBuffer = new DataBuffer(D3D11_BIND_VERTEX_BUFFER, &Vertices[0], numVertices * sizeof(Vertex));
	IndexBuffer = new DataBuffer(D3D11_BIND_INDEX_BUFFER, &Indices[0], numIndices * 2);
}

void Model::AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, Color c)
{
	Vector3f Vert[][2] =
	{
		Vector3f(x1, y2, z1), Vector3f(z1, x1), Vector3f(x2, y2, z1), Vector3f(z1, x2), // side 1
		Vector3f(x2, y2, z2), Vector3f(z2, x2), Vector3f(x1, y2, z2), Vector3f(z2, x1),
		Vector3f(x1, y1, z1), Vector3f(z1, x1), Vector3f(x2, y1, z1), Vector3f(z1, x2), // side 2
		Vector3f(x2, y1, z2), Vector3f(z2, x2), Vector3f(x1, y1, z2), Vector3f(z2, x1),
		Vector3f(x1, y1, z2), Vector3f(z2, y1), Vector3f(x1, y1, z1), Vector3f(z1, y1), // side 3
		Vector3f(x1, y2, z1), Vector3f(z1, y2), Vector3f(x1, y2, z2), Vector3f(z2, y2),
		Vector3f(x2, y1, z2), Vector3f(z2, y1), Vector3f(x2, y1, z1), Vector3f(z1, y1), // side 4
		Vector3f(x2, y2, z1), Vector3f(z1, y2), Vector3f(x2, y2, z2), Vector3f(z2, y2),
		Vector3f(x1, y1, z1), Vector3f(x1, y1), Vector3f(x2, y1, z1), Vector3f(x2, y1), // side 5
		Vector3f(x2, y2, z1), Vector3f(x2, y2), Vector3f(x1, y2, z1), Vector3f(x1, y2),
		Vector3f(x1, y1, z2), Vector3f(x1, y1), Vector3f(x2, y1, z2), Vector3f(x2, y1), // side 6
		Vector3f(x2, y2, z2), Vector3f(x2, y2), Vector3f(x1, y2, z2), Vector3f(x1, y2),
	};

	// each number is the SkyboxVertex number
	uint16_t CubeIndices[] = {
		0, 1, 3, // side 1
		3, 1, 2,
		5, 4, 6, // side 2
		6, 4, 7,
		8, 9, 11,  // side 3
		11, 9, 10,
		13, 12, 14, // side 4
		14, 12, 15,
		16, 17, 19, // side 5
		19, 17, 18,
		21, 20, 22, // side 6
		22, 20, 23
	};

	for (int i = 0; i < 36; i++)
		AddIndex(CubeIndices[i] + (uint16_t)numVertices);

	for (int v = 0; v < 24; v++)
	{
		Vertex vvv; 
		vvv.Pos = Vert[v][0]; 
		vvv.U = Vert[v][1].x; 
		vvv.V = Vert[v][1].y;
		float dist1 = (vvv.Pos - Vector3f(-2, 4, -2)).Length();
		float dist2 = (vvv.Pos - Vector3f(3, 4, -3)).Length();
		float dist3 = (vvv.Pos - Vector3f(-4, 3, 25)).Length();
		int   bri = rand() % 160;
		float RRR = c.R * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
		float GGG = c.G * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
		float BBB = c.B * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
		vvv.C.R = RRR > 255 ? 255 : (unsigned char)RRR;
		vvv.C.G = GGG > 255 ? 255 : (unsigned char)GGG;
		vvv.C.B = BBB > 255 ? 255 : (unsigned char)BBB;
		AddVertex(vvv);
	}
}


