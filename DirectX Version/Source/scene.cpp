#include "scene.h"

D3D11_INPUT_ELEMENT_DESC ModelVertexDesc[] =
{ { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Model::Vertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Model::Vertex, C), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Model::Vertex, U), D3D11_INPUT_PER_VERTEX_DATA, 0 }, };

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

char* VertexShaderSrc =
"float4x4 Proj, View;"
"void main(in  float4 Position  : POSITION,    in  float4 Color : COLOR0, in  float2 TexCoord  : TEXCOORD0,"
"          out float4 oPosition : SV_Position, out float4 oColor: COLOR0, out float2 oTexCoord : TEXCOORD0)"
"{   oPosition = mul(Proj, mul(View, Position)); oTexCoord = TexCoord; oColor = Color; }";
char* PixelShaderSrc =
"Texture2D Texture   : register(t0); SamplerState Linear : register(s0); "
"float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float2 TexCoord : TEXCOORD0) : SV_Target"
"{   return Color * Texture.Sample(Linear, TexCoord); }";

char* VertexShaderSphere =
"float4x4 WVP;"
"void main(in float3 Position : POSITION, in float3 normal : NORMAL, in float2 inTexCoord : TEXCOORD,"
"	out float4 Pos : SV_POSITION, out float3 texCoord : TEXCOORD)"
" { Pos = mul(float4(Position, 1.0f), WVP).xyww; texCoord = Position;} ";
char* PixelShaderSphere =
"TextureCube textureCube : register(t0); SamplerState ObjSamplerState : register(s0);"
"float4 main(in float4 Pos : SV_POSITION, in float3 texCoord : TEXCOORD) : SV_Target"
"{ return textureCube.Sample(ObjSamplerState, texCoord); }";

float monitorHeight = 1;
float monitorWidth = 2;
//used in addmonitor and initial
startFloat startingPoint(-0.5, 1, 1, 0.5, 2, 1, Model::Color(128, 128, 128));

void Scene::Add(Model * n)
{
	Models[num_models++] = n;
}

Scene::Scene() : num_models(0) // Main world
{

	Vector3f monitorOffset = Vector3f(0, 0, 0);

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

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/skymapSunny.dds", 0U, D3D11_USAGE_DEFAULT, 
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);
	
	ID3D11Texture2D* tex2d;
	
	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	ImageBuffer* t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);
	
	generated_texture[4] = new ShaderFill(layout, 3, VertexShaderSphere, PixelShaderSphere, t);

	// skybox
	Model * m = new Model(Vector3f(0, 0, 0), generated_texture[4]); 
	m->CreateSphere(10,10);
	Add(m);

	// Construct geometry
	// first gives the starting x y and z coordinantes then the ending x y and z coordinantes of the box and then the initial color of the model

	m = new Model(Vector3f(0, 0, 0), generated_texture[1]); // eventually will be the monitor
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);//starting details can be managed at top
	m->AllocateBuffers(); Add(m);

}

void Scene::Render(Matrix4f view, Matrix4f proj)
{
	for (int i = 0; i < num_models; i++)
	{
		Matrix4f modelmat = Models[i]->GetMatrix();
		Matrix4f mat = (view * modelmat).Transposed();

		// use only if using the sphere
		if (i == 0) {
			Matrix4f sphereWorld = sphereWorld.Identity();

			//Define sphereWorld's world space matrix
			Matrix4f Scale = Scale.Scaling(5.0f, 5.0f, 5.0f);
			//Make sure the sphere is always centered around camera
			Matrix4f Trans = Matrix4f();
			
			Trans.SetTranslation(view.GetTranslation());

			//Set sphereWorld's world space using the transformations
			sphereWorld = Scale * Trans;

			Matrix4f WVP = sphereWorld * view * proj;
			WVP = WVP.Transposed();
			Models[i]->Fill->VShader->SetUniform("WVP", 16, (float *)&WVP);
			DX11.Render(Models[i]->Fill, Models[i]->VertexBuffer, Models[i]->IndexBuffer,
				sizeof(Model::SkyboxVertex), Models[i]->NumSphereFaces * 3);
		} 
		else {

			Models[i]->Fill->VShader->SetUniform("View", 16, (float *)&mat);
			Models[i]->Fill->VShader->SetUniform("Proj", 16, (float *)&proj);
			DX11.Render(Models[i]->Fill, Models[i]->VertexBuffer, Models[i]->IndexBuffer,
				sizeof(Model::Vertex), Models[i]->numIndices);
		
			}
	
}

}
Vector3f Scene::getLastMonitorPosition(){
	return Models[num_models-1]->Pos;
}

Vector3f Scene::getOffset(){
	//TODO
	return monitorOffset;
}

void Scene::setOffset(){
	monitorOffset += Vector3f(0, monitorHeight, 0);
}

void Scene::addMonitor(){
	static Model::Color tex_pixels[4][256 * 256];
	//we would probably fill this tex with pixels from desktop dup here
	ImageBuffer* t = new ImageBuffer(true, true, Sizei(256, 256), 8, (unsigned char *)tex_pixels); // eventually will be the monitor
	ShaderFill * generated_texture = new ShaderFill(ModelVertexDesc, 3, VertexShaderSrc, PixelShaderSrc, t);
	Vector3f temp = getLastMonitorPosition();
	Vector3f tempVect = Vector3f(2, 0, 0) + getLastMonitorPosition();//initialize in case we change in loop below
	if (num_models % 3 == 0){//reset the position of the next monitor if they wont fit on screen
		
		temp.x = startingPoint.x1;
		temp.y = startingPoint.y1 + monitorHeight;
		temp.z = startingPoint.z1;
		tempVect = temp;
	}
	
	Model* m = new Model(tempVect, generated_texture);
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);
	m->AllocateBuffers(); Add(m);
}