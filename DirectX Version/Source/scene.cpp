#include "scene.h"




ID3D11ShaderResourceView* smrv;
ID3D11DepthStencilState* DSLessEqual;
ID3D11RasterizerState* RSCullNone;



void	Scene::Add(Model * n)
{
	Models[num_models++] = n;
}

Scene::Scene(int reducedVersion) : num_models(0) // Main world
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
	//	CreateSphere(10, 10);

	// Construct textures
	static Model::Color tex_pixels[4][256 * 256];
	ShaderFill * generated_texture[5];

	// this is what makes the checkered floors and walls textures 
	for (int k = 0; k<4; k++)
	{
		for (int j = 0; j < 256; j++)
			for (int i = 0; i < 256; i++)
			{
				// these all alternate back and forth between colors giving them the checkered textures
				if (k == 0) tex_pixels[0][j * 256 + i] = (((i >> 7) ^ (j >> 7)) & 1) ? Model::Color(180, 180, 180, 255) : Model::Color(80, 80, 80, 255);// floor
				if (k == 1) tex_pixels[1][j * 256 + i] = (((j / 4 & 15) == 0) || (((i / 4 & 15) == 0) && ((((i / 4 & 31) == 0) ^ ((j / 4 >> 4) & 1)) == 0))) ?
					Model::Color(60, 60, 60, 255) : Model::Color(180, 180, 180, 255); //wall
				if (k == 2) tex_pixels[2][j * 256 + i] = (i / 4 == 0 || j / 4 == 0) ? Model::Color(80, 80, 80, 255) : Model::Color(180, 180, 180, 255);// ceiling
				if (k == 3) tex_pixels[3][j * 256 + i] = Model::Color(128, 128, 128, 255);// blank
			}
		// load the finished texture pixels into the image buffer
		ImageBuffer * t = new ImageBuffer(false, false, Sizei(256, 256), 8, (unsigned char *)tex_pixels[k]);
		// then create these textures into shaders
		generated_texture[k] = new ShaderFill(ModelVertexDesc, 3, VertexShaderSrc, PixelShaderSrc, t);
	}

	ID3D11Resource* resource;
	ID3D11ShaderResourceView* shaderResource;

	CreateDDSTextureFromFile(DX11.Device, L"Assets/skybox.dds", &resource, &shaderResource);//if skyboxCubeMapDDS is used the image is warped in all directions, but only shows one "face" of the cubemap on all sides

	ID3D11Texture2D* tex2d;

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	D3D11_TEXTURE2D_DESC SMTextureDesc;
	tex2d->GetDesc(&SMTextureDesc);

	ImageBuffer* t = new ImageBuffer(true, true, Sizei(256, 256), 8, tex2d, shaderResource);
	generated_texture[4] = new ShaderFill(ModelVertexDesc, 3, VertexShaderSrc, PixelShaderSrc, t);




	//setup Culling
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	//i just left these below for example
	//	cmdesc.CullMode = D3D11_CULL_BACK;
	//	cmdesc.FrontCounterClockwise = true;
	//	DX11.Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

	//	cmdesc.FrontCounterClockwise = false;

	//	DX11.Device->CreateRasterizerState(&cmdesc, &CWcullMode);
	//skymap render states
	//-----------------------
	cmdesc.CullMode = D3D11_CULL_NONE;
	DX11.Device->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	DX11.Device->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	//set culling none, shows both sides
	DX11.Context->RSSetState(RSCullNone);//if this is commented out the cube will be invisible from the inside but visible on the outside


	// Construct geometry
	// first gives the starting x y and z coordinantes then the ending x y and z coordinantes of the box and then the initial color of the model

	Model * m = new Model(Vector3f(0, 0, 0), generated_texture[4]); // eventually will be skybox
	m->AddSolidColorBox(-3, -3, -3, 3, 3, 3, Model::Color(128, 128, 128));
	m->AllocateBuffers(); Add(m);

	m = new Model(Vector3f(0, 0, 0), generated_texture[1]); // eventually will be the monitor
	m->AddSolidColorBox(-1, 1, 2, 1, 2, 2, Model::Color(128, 128, 128));
	m->AllocateBuffers(); Add(m);

}

Scene::Scene() : num_models(0)
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

void Scene::Render(Matrix4f view, Matrix4f proj)
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

