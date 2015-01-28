#ifndef SCENE
#define SCENE

#include "Win32_DX11AppUtil.h"
#include "model.h"

//------------------------------------------------------------------------- 
struct Scene
{
	int     num_models;
	Model * Models[10];

	void    Add(Model * n)
	{
		Models[num_models++] = n;
	}

	Scene(int reducedVersion) : num_models(0) // Main world
	{
		D3D11_INPUT_ELEMENT_DESC ModelVertexDesc[] =
		{ { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Model::Vertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Model::Vertex, C), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Model::Vertex, U), D3D11_INPUT_PER_VERTEX_DATA, 0 }, };

		char* VertexShaderSrc =
			"float4x4 Proj, View;"
			"float4 NewCol;"
			"void main(in  float4 Position  : POSITION,    in  float4 Color : COLOR0, in  float2 TexCoord  : TEXCOORD0,"
			"          out float4 oPosition : SV_Position, out float4 oColor: COLOR0, out float2 oTexCoord : TEXCOORD0)"
			"{   oPosition = mul(Proj, mul(View, Position)); oTexCoord = TexCoord; oColor = Color; }";
		char* PixelShaderSrc =
			"Texture2D Texture   : register(t0); SamplerState Linear : register(s0); "
			"float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float2 TexCoord : TEXCOORD0) : SV_Target"
			"{   return Color * Texture.Sample(Linear, TexCoord); }";

		// Construct textures
		static Model::Color tex_pixels[4][256 * 256];
		ShaderFill * generated_texture[4];

		for (int k = 0; k<4; k++)
		{
			for (int j = 0; j < 256; j++)
			for (int i = 0; i < 256; i++)
			{
				if (k == 0) tex_pixels[0][j * 256 + i] = (((i >> 7) ^ (j >> 7)) & 1) ? Model::Color(180, 180, 180, 255) : Model::Color(80, 80, 80, 255);// floor
				if (k == 1) tex_pixels[1][j * 256 + i] = (((j / 4 & 15) == 0) || (((i / 4 & 15) == 0) && ((((i / 4 & 31) == 0) ^ ((j / 4 >> 4) & 1)) == 0))) ?
					Model::Color(60, 60, 60, 255) : Model::Color(180, 180, 180, 255); //wall
				if (k == 2) tex_pixels[2][j * 256 + i] = (i / 4 == 0 || j / 4 == 0) ? Model::Color(80, 80, 80, 255) : Model::Color(180, 180, 180, 255);// ceiling
				if (k == 3) tex_pixels[3][j * 256 + i] = Model::Color(128, 128, 128, 255);// blank
			}
			ImageBuffer * t = new ImageBuffer(false, false, Sizei(256, 256), 8, (unsigned char *)tex_pixels[k]);
			generated_texture[k] = new ShaderFill(ModelVertexDesc, 3, VertexShaderSrc, PixelShaderSrc, t);
		}
		// Construct geometry
		Model * m = new Model(Vector3f(0, 0, 0), generated_texture[2]);  // Moving box
		m->AddSolidColorBox(0, 0, 0, +1.0f, +1.0f, 1.0f, Model::Color(64, 64, 64));
		m->AllocateBuffers(); Add(m);

		m = new Model(Vector3f(0, 0, 0), generated_texture[1]);  // Walls
		m->AddSolidColorBox(-10.1f, 0.0f, -20.0f, -10.0f, 4.0f, 20.0f, Model::Color(128, 128, 128)); // Left Wall
		m->AddSolidColorBox(-10.0f, -0.1f, -20.1f, 10.0f, 4.0f, -20.0f, Model::Color(128, 128, 128)); // Back Wall
		m->AddSolidColorBox(10.0f, -0.1f, -20.0f, 10.1f, 4.0f, 20.0f, Model::Color(128, 128, 128));  // Right Wall
		m->AllocateBuffers(); Add(m);

		m = new Model(Vector3f(0, 0, 0), generated_texture[0]);  // Floors
		m->AddSolidColorBox(-10.0f, -0.1f, -20.0f, 10.0f, 0.0f, 20.1f, Model::Color(128, 128, 128)); // Main floor
		m->AddSolidColorBox(-15.0f, -6.1f, 18.0f, 15.0f, -6.0f, 30.0f, Model::Color(128, 128, 128));// Bottom floor
		m->AllocateBuffers(); Add(m);

		if (reducedVersion) return;

		m = new Model(Vector3f(0, 0, 0), generated_texture[2]);  // Ceiling
		m->AddSolidColorBox(-10.0f, 4.0f, -20.0f, 10.0f, 4.1f, 20.1f, Model::Color(128, 128, 128));
		m->AllocateBuffers(); Add(m);

		m = new Model(Vector3f(0, 0, 0), generated_texture[3]);  // Fixtures & furniture
		m->AddSolidColorBox(9.5f, 0.75f, 3.0f, 10.1f, 2.5f, 3.1f, Model::Color(96, 96, 96));   // Right side shelf// Verticals
		m->AddSolidColorBox(9.5f, 0.95f, 3.7f, 10.1f, 2.75f, 3.8f, Model::Color(96, 96, 96));   // Right side shelf
		m->AddSolidColorBox(9.55f, 1.20f, 2.5f, 10.1f, 1.30f, 3.75f, Model::Color(96, 96, 96)); // Right side shelf// Horizontals
		m->AddSolidColorBox(9.55f, 2.00f, 3.05f, 10.1f, 2.10f, 4.2f, Model::Color(96, 96, 96)); // Right side shelf
		m->AddSolidColorBox(5.0f, 1.1f, 20.0f, 10.0f, 1.2f, 20.1f, Model::Color(96, 96, 96));   // Right railing   
		m->AddSolidColorBox(-10.0f, 1.1f, 20.0f, -5.0f, 1.2f, 20.1f, Model::Color(96, 96, 96));   // Left railing  
		for (float f = 5.0f; f <= 9.0f; f += 1.0f)
		{
			m->AddSolidColorBox(f, 0.0f, 20.0f, f + 0.1f, 1.1f, 20.1f, Model::Color(128, 128, 128));// Left Bars
			m->AddSolidColorBox(-f, 1.1f, 20.0f, -f - 0.1f, 0.0f, 20.1f, Model::Color(128, 128, 128));// Right Bars
		}
		m->AddSolidColorBox(-1.8f, 0.8f, 1.0f, 0.0f, 0.7f, 0.0f, Model::Color(128, 128, 0)); // Table
		m->AddSolidColorBox(-1.8f, 0.0f, 0.0f, -1.7f, 0.7f, 0.1f, Model::Color(128, 128, 0)); // Table Leg 
		m->AddSolidColorBox(-1.8f, 0.7f, 1.0f, -1.7f, 0.0f, 0.9f, Model::Color(128, 128, 0)); // Table Leg 
		m->AddSolidColorBox(0.0f, 0.0f, 1.0f, -0.1f, 0.7f, 0.9f, Model::Color(128, 128, 0)); // Table Leg 
		m->AddSolidColorBox(0.0f, 0.7f, 0.0f, -0.1f, 0.0f, 0.1f, Model::Color(128, 128, 0)); // Table Leg 
		m->AddSolidColorBox(-1.4f, 0.5f, -1.1f, -0.8f, 0.55f, -0.5f, Model::Color(44, 44, 128)); // Chair Set
		m->AddSolidColorBox(-1.4f, 0.0f, -1.1f, -1.34f, 1.0f, -1.04f, Model::Color(44, 44, 128)); // Chair Leg 1
		m->AddSolidColorBox(-1.4f, 0.5f, -0.5f, -1.34f, 0.0f, -0.56f, Model::Color(44, 44, 128)); // Chair Leg 2
		m->AddSolidColorBox(-0.8f, 0.0f, -0.5f, -0.86f, 0.5f, -0.56f, Model::Color(44, 44, 128)); // Chair Leg 2
		m->AddSolidColorBox(-0.8f, 1.0f, -1.1f, -0.86f, 0.0f, -1.04f, Model::Color(44, 44, 128)); // Chair Leg 2
		m->AddSolidColorBox(-1.4f, 0.97f, -1.05f, -0.8f, 0.92f, -1.10f, Model::Color(44, 44, 128)); // Chair Back high bar

		for (float f = 3.0f; f <= 6.6f; f += 0.4f)
			m->AddSolidColorBox(-3, 0.0f, f, -2.9f, 1.3f, f + 0.1f, Model::Color(64, 64, 64));//Posts

		m->AllocateBuffers(); Add(m);
	}

	// Simple latency box (keep similar vertex format and shader params same, for ease of code)
	Scene() : num_models(0)
	{
		D3D11_INPUT_ELEMENT_DESC ModelVertexDesc[] =
		{ { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Model::Vertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }, };

		char* VertexShaderSrc =
			"float4x4 Proj, View;"
			"float4 NewCol;"
			"void main(in float4 Position : POSITION, out float4 oPosition : SV_Position, out float4 oColor: COLOR0)"
			"{   oPosition = mul(Proj, Position); oColor = NewCol; }";
		char* PixelShaderSrc =
			"float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0) : SV_Target"
			"{   return Color ; }";

		Model* m = new Model(Vector3f(0, 0, 0), new ShaderFill(ModelVertexDesc, 3, VertexShaderSrc, PixelShaderSrc, 0));
		float scale = 0.04f;  float extra_y = ((float)DX11.WinSize.w / (float)DX11.WinSize.h);
		m->AddSolidColorBox(1 - scale, 1 - (scale*extra_y), -1, 1 + scale, 1 + (scale*extra_y), -1, Model::Color(0, 128, 0));
		m->AllocateBuffers(); Add(m);
	}

	void Render(Matrix4f view, Matrix4f proj)
	{
		for (int i = 0; i < num_models; i++)
		{
			Matrix4f modelmat = Models[i]->GetMatrix();
			Matrix4f mat = (view * modelmat).Transposed();

			Models[i]->Fill->VShader->SetUniform("View", 16, (float *)&mat);
			Models[i]->Fill->VShader->SetUniform("Proj", 16, (float *)&proj);

			DX11.Render(Models[i]->Fill, Models[i]->VertexBuffer, Models[i]->IndexBuffer,
				sizeof(Model::Vertex), Models[i]->numIndices);
		}
	}
};

#endif
