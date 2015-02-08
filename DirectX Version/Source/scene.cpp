#include "scene.h"
#include "desktop.h"

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

char * monitorVertes =
"struct VS_INPUT{float4 Pos : POSITION;float2 Tex : TEXCOORD;};"
"struct VS_OUTPUT{float4 Pos : SV_POSITION;float2 Tex : TEXCOORD;};"
"VS_OUTPUT main(VS_INPUT input){return input;}";

char * monitorPixels =
"Texture2D tx : register(t0); SamplerState samLinear : register(s0);"
"struct PS_INPUT{ float4 Pos : SV_POSITION; float2 Tex : TEXCOORD; };"
"float4 main(PS_INPUT input) : SV_Target{ return tx.Sample(samLinear, input.Tex); }";

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


//used in addmonitor and initialization just below here for the first "screen"
startFloat startingPoint(-2.0, 0, 1, 2, 4, 1, Model::Color(128, 128, 128));

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

    //TODO: add desktop duplication


	// skybox
	Model * m = new Model(Vector3f(0, 0, 0), generated_texture[4]); 
	m->CreateSphere(10,10);
	Add(m);

    addMonitor();
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


//This is set in scene.cpp
Vector3f Scene::getOffset(){
	//TODO
	return monitorOffset;
}

//sets the monitor offset used to separate monitors
void Scene::setOffset(Vector3f _Voffset){
	monitorOffset = _Voffset;
}

void Scene::addMonitor(){

    // first gives the starting x y and z coordinantes then the ending x y and z coordinantes of the box and then the initial color of the model
    Desktop * desktop = new Desktop();
    desktop->init();
    FRAME_DATA data;
    bool timed;
    desktop->getFrame(&data, &timed);

    // The way the final prototype will work is that there will be one master surface that is rendered
    // and then a second disposable surface which will map dirty bits onto the new one. 

    D3D11_TEXTURE2D_DESC DeskTexD;
    desktop->desktopImage->GetDesc(&DeskTexD);
    DeskTexD.MipLevels = 1;
    DeskTexD.ArraySize = 1;
    DeskTexD.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    DeskTexD.SampleDesc.Count = 1;
    DeskTexD.SampleDesc.Quality = 0;
    DeskTexD.Usage = D3D11_USAGE_STAGING;
    DeskTexD.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    DeskTexD.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    DeskTexD.MiscFlags = 0;
    ID3D11Texture2D* masterTex;
    DX11.Device->CreateTexture2D(&DeskTexD, NULL, &masterTex);

    ID3D11ShaderResourceView* ShaderResource = nullptr;
    DX11.Device->CreateShaderResourceView(masterTex, NULL, &ShaderResource);
    desktop->deviceContext->CopyResource(masterTex, desktop->desktopImage);

    ImageBuffer* w = new ImageBuffer(true, false, Sizei(DeskTexD.Width, DeskTexD.Height), masterTex, shaderResource);
    D3D11_INPUT_ELEMENT_DESC Layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    ShaderFill * generated_texture = new ShaderFill(Layout, 3, monitorVertes, monitorPixels, w);
	
	//if we have two monitors on the bottom then we need to add some up top
	//so get where we started from and add an offset, only supports about 5 monitors total right now
	Vector3f temp = getLastMonitorPosition();
	Vector3f tempVect = getOffset() + getLastMonitorPosition();//initialize in case we change in loop below
	if (num_models % 3 == 0){//reset the position of the next monitor if they wont fit on screen	
		//so go back to the initial point on x, add an offset to put them on top 
		temp.x = startingPoint.x1;
		temp.y = startingPoint.y1 + monitorHeight/2;
		temp.z = startingPoint.z1;//and reset z so when we get the last monitor position it doesnt start pushing them back
		tempVect = temp;//reset tempVect to this one since we had to reposition
	}
	
	//set the new model with the repositioned ones above
	Model* m = new Model(tempVect, generated_texture);
	//everything is added based on the first monitor the startingpoint monitor
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);
	m->AllocateBuffers(); Add(m);
}