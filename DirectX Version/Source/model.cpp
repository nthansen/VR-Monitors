#include "model.h"

Model::Model(Vector3f arg_pos, ShaderFill * arg_Fill) 
{ 
	numVertices = 0; numIndices = 0; 
	Pos = arg_pos; 
	Fill = arg_Fill; 
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

void Model::AllocateBuffers()
{
	VertexBuffer = new DataBuffer(D3D11_BIND_VERTEX_BUFFER, &Vertices[0], numVertices * sizeof(Vertex));
	IndexBuffer = new DataBuffer(D3D11_BIND_INDEX_BUFFER, &Indices[0], numIndices * 2);
}




void Model::AddSkybox()
{
	VertexPos Vert[] =
	{
		{ Vector3f(1.0f, 1.0f, 1.0f), Vector2f(1.0f, 1.0f) },
		{ Vector3f(1.0f, -1.0f, 1.0f), Vector2f(1.0f, 0.0f) },
		{ Vector3f(-1.0f, -1.0f, 1.0f), Vector2f(0.0f, 0.0f) },
		{ Vector3f(-1.0f, -1.0f, 1.0f), Vector2f(0.0f, 0.0f) },
		{ Vector3f(-1.0f, 1.0f, 1.0f), Vector2f(0.0f, 1.0f) },
		{ Vector3f(1.0f, 1.0f, 1.0f), Vector2f(1.0f, 1.0f) },
	};


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
		Vertex vvv; vvv.Pos = Vert[v][0]; vvv.U = Vert[v][1].x; vvv.V = Vert[v][1].y;
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


void Model::CreateSphere(int LatLines, int LongLines)
{
	NumSphereVertices = ((LatLines - 2) * LongLines) + 4;
	NumSphereVertices *= 2;
	NumSphereFaces = ((LatLines - 3)*(LongLines)* 2) + (LongLines * 2);
	NumSphereFaces *= 2;

	Rotationx = XMMATRIX();
	Rotationy = XMMATRIX();
	Rotationz = XMMATRIX();

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	vector<SkyboxVertex> vertices(NumSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	// for the front part of the sphere
	for (DWORD i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (3.14 / (LatLines - 1));
		Rotationx = XMMatrixRotationX(spherePitch);
		for (DWORD j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (6.28 / (LongLines));
			Rotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	// for the back part of the sphere
	for (DWORD i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (3.14 / (LatLines - 1));
		Rotationx = XMMatrixRotationX(spherePitch);
		for (DWORD j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (6.28 / (LongLines));
			Rotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].pos.x = -XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].pos.y = -XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].pos.z = -XMVectorGetZ(currVertPos);
		}
	}

	// if enabled causes a texture
	//vertices[NumSphereVertices - 1].pos.x = 0.0f;
	//vertices[NumSphereVertices - 1].pos.y = 0.0f;
	//vertices[NumSphereVertices - 1].pos.z = -1.0f;

	VertexBuffer = new DataBuffer(D3D11_BIND_VERTEX_BUFFER, &vertices[0], sizeof(SkyboxVertex)* NumSphereVertices);

	std::vector<DWORD> indices(NumSphereFaces * 3);

	int k = 0;
	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = LongLines;
	indices[k + 2] = 1;
	k += 3;

	for (DWORD i = 0; i < LatLines - 3; ++i)
	{
		for (DWORD j = 0; j < LongLines - 1; ++j)
		{
			indices[k] = i*LongLines + j + 1;
			indices[k + 1] = i*LongLines + j + 2;
			indices[k + 2] = (i + 1)*LongLines + j + 1;

			indices[k + 3] = (i + 1)*LongLines + j + 1;
			indices[k + 4] = i*LongLines + j + 2;
			indices[k + 5] = (i + 1)*LongLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*LongLines) + LongLines;
		indices[k + 1] = (i*LongLines) + 1;
		indices[k + 2] = ((i + 1)*LongLines) + LongLines;

		indices[k + 3] = ((i + 1)*LongLines) + LongLines;
		indices[k + 4] = (i*LongLines) + 1;
		indices[k + 5] = ((i + 1)*LongLines) + 1;

		k += 6;
	}

	for (DWORD l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = NumSphereVertices - 1;
		indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
		indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = NumSphereVertices - 1;
	indices[k + 1] = (NumSphereVertices - 1) - LongLines;
	indices[k + 2] = NumSphereVertices - 2;



	IndexBuffer = new DataBuffer(D3D11_BIND_INDEX_BUFFER, &indices[0], sizeof(DWORD)* NumSphereFaces * 3);

}


