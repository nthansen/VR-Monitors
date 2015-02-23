#include "scene.h"

D3D11_INPUT_ELEMENT_DESC ModelVertexDesc[] =
{ { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Model::Vertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Model::Vertex, C), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Model::Vertex,U), D3D11_INPUT_PER_VERTEX_DATA, 0 }, };
const float PI = 3.1415927f;

//used in addmonitor and initialization just below here for the first "screen"
//startFloat startingPoint(-0.5, -0.5, 0.8, 0.5, 0.5, 0.8, Model::Color(128, 128, 128));

void Scene::Add(Model * n)
{
	Models[num_models++] = n;
}

int Scene::pickMonitor(Vector3f Pos, float Yaw){
	bool hit = false;//stores whether we hit something
	//Model *target = nullptr;//model we will find
	//save the direction to cast in dir and position to start from in dpos
	SimpleMath::Vector3 dPos = SimpleMath::Vector3(Pos.x, Pos.y, Pos.z);//position for simplemath
	SimpleMath::Vector3 dir = SimpleMath::Vector3(sinf(Yaw-PI), 0, cosf(Yaw-PI));
	//ray is really only a container holding the origin and direction to cast
	SimpleMath::Ray cast = SimpleMath::Ray(dPos, dir);
	//crate a bounding box around each monitor to see if we hit one
	//as of now the second parameter is simply the distance from the center of the box to each edge x,y,z,w
	//magic number .4f is the distance from the center in x,y,z boxes center will be different
	//DirectX::BoundingOrientedBox mbox = BoundingOrientedBox(XMFLOAT3(0, 0, 1), XMFLOAT3(.4f, .4f, 0.0f), XMFLOAT4(sinf(Yaw), 0, cosf(Yaw - PI), 0));
	int i = 0;
	float _dist = 0.0f;
	for (i; i < num_monitors; i++){
		Model *temp = Monitors[i];
		DirectX::BoundingOrientedBox mbox = BoundingOrientedBox(XMFLOAT3(temp->Pos.x, temp->Pos.y, temp->Pos.z), XMFLOAT3(.5f, .5f, 0), XMFLOAT4(0, temp->Rot.y, 0, temp->Rot.w));
		DirectX::BoundingSphere mSphere = BoundingSphere(XMFLOAT3(temp->Pos.x, temp->Pos.y, temp->Pos.z), monitorHeight);
		//Model * m = new Model(Vector3f(0, 0, startingPoint.z1), generated_texture[1]); // for bounding box
		//m->AddSolidColorBox(temp->Pos.x, temp->Pos.y, temp->Pos.z, temp->Pos.x + monitorWidth ,
		//	temp->Pos.y + monitorHeight, temp->Pos.z + monitorDepth, Model::Color(0,128,128,1));//to visualize the selected monitor
		//m->AllocateBuffers(); Add(m); selected = m;
		if (mbox.Intersects(cast.position, cast.direction, _dist)){
			//					*target = *temp; //causes error but may want to save reference
			//loop causes freeze need to get a reference and keep moving the monitor someplace
			//while (controlPanel.movingMonitor){
			/*temp->Pos = Pos;
			temp->Rot = Quatf(Vector3f(0, .001, 0), -PI + Yaw);*/
			//}
			//so for now well just return the model
			/*return temp;*/
			hit = true;
			return i;
		}
		if (hit){
			int j = 0; //just checking for testing
		}
	}
	return 0;//return default first monitor


}

// Main world
Scene::Scene() :
num_models(0), num_monitors(0), monitorHeight(1), monitorWidth(1), monitorDepth(0.8f),

startingPoint(0-monitorWidth/2,0-monitorHeight/2,monitorDepth,monitorWidth/2,monitorHeight/2,monitorDepth,
Model::Color(128, 128, 128))
{
	
	Vector3f monitorOffset = Vector3f(0, 0, 0);

	// Construct textures
	static Model::Color tex_pixels[4][256 * 256];

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
		generated_texture[k] = new ShaderFill(ModelVertexDesc, 3, Box, t);
	}

	loadSkyboxes();

	// Construct geometry
	// first gives the starting x y and z coordinantes then the ending x y and z coordinantes of the box and then the initial color of the model

	//add first monitor
	Model * m = new Model(Vector3f(0, 0, startingPoint.z1), generated_texture[1]); // eventually will be the monitor
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);//starting details can be managed at top
	m->AllocateBuffers(); Add(m); Monitors[num_monitors++] = m; selected = m;

	// skybox
	m = new Model(Vector3f(0, 0, 0), generated_texture[4]);
	m->AddSolidColorBox(-10, -10, -10, 10, 10, 10, Model::Color(128, 128, 128));
	m->AllocateBuffers();
	Add(m);
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
Vector3f Scene::getLastMonitorPosition(){
	return Monitors[num_monitors-1]->Pos;
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

void Scene::addMonitor(float yaw,Vector3f _pos){
	if (num_monitors < 3) {
	static Model::Color tex_pixels[4][256 * 256];
	//we would probably fill this tex with pixels from desktop dup here
	ImageBuffer* t = new ImageBuffer(true, true, Sizei(256, 256), 8, (unsigned char *)tex_pixels); // eventually will be the monitor
	ShaderFill * generated_texture = new ShaderFill(ModelVertexDesc, 3, Box, t);
	
	num_monitors++;//add one to monitors here to determine how to group them below
	Model* m = new Model(Vector3f(0, 0, startingPoint.z1), generated_texture);
	//everything is added based on the first monitor the startingpoint monitor
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);
	m->AllocateBuffers(); Add(m);//add monitor to scene array to be rendered;
		Monitors[num_monitors - 1] = m;//add this monitor to the array of monitors	//if we have two monitors on the bottom then we need to add some up top
	//so get where we started from and add an offset, only supports about 5 monitors total right now
	//Vector3f temp = getLastMonitorPosition();
	//Vector3f tempVect = getOffset() + getLastMonitorPosition();//initialize in case we change in loop below
	if (num_monitors == 2){//change position of first monitor
		Monitors[0]->Pos = _pos;
		Monitors[0]->OriginalPos = _pos;
		//Monitors[0]->Pos.x = ;//do i need to shift the monitor position?
			Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw - PI / 5.5);
		Monitors[0]->OriginalRot = Monitors[0]->Rot;
		Monitors[1]->Pos = _pos;
		Monitors[1]->OriginalPos = _pos;
		Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw + PI / 5.5);
		Monitors[1]->OriginalRot = Monitors[1]->Rot;
		//so go back to the initial point on x, add an offset to put them on top 
		//temp.x = startingPoint.x1;
		//temp.y = startingPoint.y1 + monitorHeight/2;
		//temp.z = startingPoint.z1;//and reset z so when we get the last monitor position it doesnt start pushing them back
		//tempVect = temp;//reset tempVect to this one since we had to reposition
	}
	else if (num_monitors == 3){
		Monitors[0]->Pos = _pos;
		Monitors[0]->OriginalPos = _pos;
			Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw);
		Monitors[0]->OriginalRot = Monitors[0]->Rot;
		Monitors[1]->Pos = _pos;
		Monitors[1]->OriginalPos = _pos;
		Monitors[1]->Pos += Vector3f(monitorWidth/5, 0, -monitorDepth/6);//move left monitor out by a factor of width
		Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw + PI / 3.5);
		Monitors[1]->OriginalRot = Monitors[1]->Rot;
		Monitors[2]->Pos = _pos;
		Monitors[2]->OriginalPos = _pos;
		Monitors[2]->Pos += Vector3f(-monitorWidth / 5, 0, -monitorDepth/6);//move left monitor out by a factor of width

		Monitors[2]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw - PI / 3.5);
		Monitors[2]->OriginalRot = Monitors[2]->Rot;
	}
	}
	
	////set the new model with the repositioned ones above
	//Model* m = new Model(Vector3f(0,0,startingPoint.z1), generated_texture);
	////everything is added based on the first monitor the startingpoint monitor
	//m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
	//	startingPoint.y2, startingPoint.z2, startingPoint.color);
	//m->AllocateBuffers(); Add(m);//add monitor to scene array to be rendered;
	//Monitors[num_monitors] = m;//add this monitor to the array of monitors
}

// loads the skyboxes into the image buffers to be used with the dropdown menu and allow users to change skybox images
void Scene::loadSkyboxes() {

	ID3D11Resource* resource;
	ID3D11ShaderResourceView* shaderResource;
	ID3D11Texture2D* tex2d;

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/Evening.dds", 0U, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	ImageBuffer* t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);

	generated_texture[4] = new ShaderFill(ModelVertexDesc, 3, Skybox, t);

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/Sunny.dds", 0U, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);

	generated_texture[5] = new ShaderFill(ModelVertexDesc, 3, Skybox, t);

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/Pier.dds", 0U, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);

	generated_texture[6] = new ShaderFill(ModelVertexDesc, 3, Skybox, t);

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/Beach.dds", 0U, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);

	generated_texture[7] = new ShaderFill(ModelVertexDesc, 3, Skybox, t);

	CreateDDSTextureFromFileEx(DX11.Device, L"Assets/Beach2.dds", 0U, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, true, &resource, &shaderResource);

	resource->QueryInterface(IID_ID3D11Texture2D, (void **)&tex2d);

	t = new ImageBuffer(true, true, Sizei(256, 256), tex2d, shaderResource);

	generated_texture[8] = new ShaderFill(ModelVertexDesc, 3, Skybox, t);

}