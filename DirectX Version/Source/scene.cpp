#include "scene.h"
#include "desktop.h"


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

// Main world
Scene::Scene() :
num_models(0), num_monitors(0), monitorHeight(2), monitorWidth(2), monitorDepth(0.5f),

startingPoint(0,0,monitorDepth,monitorWidth,monitorHeight,monitorDepth,
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

	Desktop * desktop = new Desktop();
	// to signal that we are doing this for the main monitor
	desktop->init(false);

	//add first monitor
	Model * m = new Model(Vector3f(0, 0, startingPoint.z1), desktop->masterFill); // eventually will be the monitor
	m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
		startingPoint.y2, startingPoint.z2, startingPoint.color);//starting details can be managed at top
	m->AllocateBuffers(); Add(m); Monitors[num_monitors++]=m;
	m->desktop = desktop;

	// skybox
	m = new Model(Vector3f(0, 0, 0), generated_texture[4]);
	m->AddSolidColorBox(-10, -10, -10, 10, 10, 10, Model::Color(128, 128, 128));
	m->AllocateBuffers();
	Add(m);

	Dexpot testing = Dexpot(L"testing dexpot");

	bool isDexpoRunning = testing.Connect();

	int virtualDesktops = testing.GetDesktopCount();

	CaptureAnImage(testing.GetDexpotWindow());

	m = new Model(Vector3f(0, 0, 0), generated_texture[2]);
	m->AddSolidColorBox(0, 0, -3, 2, 2, -3, Model::Color(128, 128, 128));
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
		Desktop*  d = new Desktop();
		d->init(true);

		_pos = Vector3f(0, 0, 0);

		num_monitors++;//add one to monitors here to determine how to group them below
		Model* m = new Model(Vector3f(0, 0, startingPoint.z1), d->masterFill);
		m->desktop = d;
		//everything is added based on the first monitor the startingpoint monitor
		m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
			startingPoint.y2, startingPoint.z2, startingPoint.color);
		m->AllocateBuffers(); Add(m);//add monitor to scene array to be rendered;
		Monitors[num_monitors - 1] = m;//add this monitor to the array of monitors	//if we have two monitors on the bottom then we need to add some up top
		//so get where we started from and add an offset, only supports about 5 monitors total right now
		//Vector3f temp = getLastMonitorPosition();
		//Vector3f tempVect = getOffset() + getLastMonitorPosition();//initialize in case we change in loop below
		if (num_monitors == 2){//change position of first monitor
			Monitors[1]->Pos = _pos;
			Monitors[1]->OriginalPos = _pos;
			//Monitors[0]->Pos.x = ;//do i need to shift the monitor position?
			Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw - PI / 5.5);
			Monitors[1]->OriginalRot = Monitors[1]->Rot;
			_pos = Vector3f(1, 0, monitorDepth * 2);
			Monitors[0]->Pos = _pos;
			Monitors[0]->OriginalPos = _pos;
			Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw + PI / 5.5);
			Monitors[0]->OriginalRot = Monitors[0]->Rot;
			//so go back to the initial point on x, add an offset to put them on top 
			//temp.x = startingPoint.x1;
			//temp.y = startingPoint.y1 + monitorHeight/2;
			//temp.z = startingPoint.z1;//and reset z so when we get the last monitor position it doesnt start pushing them back
			//tempVect = temp;//reset tempVect to this one since we had to reposition
		}
		else if (num_monitors == 3){
			_pos = Vector3f(0, 0, monitorDepth * 2);
			Monitors[0]->Pos = _pos;
			Monitors[0]->OriginalPos = _pos;
			Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw);
			Monitors[0]->OriginalRot = Monitors[0]->Rot;
			Monitors[1]->Pos = _pos;
			Monitors[1]->OriginalPos = _pos;
			Monitors[1]->Pos += Vector3f(monitorWidth * .77, 0, monitorDepth * .2);//move left monitor out by a factor of width
			Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw + PI / 4);
			Monitors[1]->OriginalRot = Monitors[1]->Rot;
			_pos = Vector3f(0, 0, 0);
			Monitors[2]->Pos = _pos;
			Monitors[2]->OriginalPos = _pos;
			Monitors[2]->Pos += Vector3f(-monitorWidth * .52, 0, -monitorDepth * .6);//move left monitor out by a factor of width

			Monitors[2]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw - PI / 4);
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

void Scene::CaptureAnImage(HWND hWnd){
	HDC hdcScreen;
	HDC hdcWindow;
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;

	// Retrieve the handle to a display device context for the client 
	// area of the window. 
	hdcScreen = GetDC(NULL);
	hdcWindow = GetDC(hWnd);

	// Create a compatible DC which is used in a BitBlt from the window DC
	hdcMemDC = CreateCompatibleDC(hdcWindow);

	if (!hdcMemDC)
	{
		MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
		goto done;
	}

	// Get the client area for size calculation
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	//This is the best stretch mode
	SetStretchBltMode(hdcWindow, HALFTONE);

	//The source DC is the entire screen and the destination DC is the current window (HWND)
	if (!StretchBlt(hdcWindow,
		0, 0,
		rcClient.right, rcClient.bottom,
		hdcScreen,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		SRCCOPY))
	{
		MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
		goto done;
	}

	// Create a compatible bitmap from the Window DC
	hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

	if (!hbmScreen)
	{
		MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
		goto done;
	}

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	if (!BitBlt(hdcMemDC,
		0, 0,
		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
		hdcWindow,
		0, 0,
		SRCCOPY))
	{
		MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
		goto done;
	}

	// Get the BITMAP from the HBITMAP
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
	char *lpbitmap = (char *)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	GetDIBits(hdcWindow, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	// A file is created, this is where we will save the screen capture.
	HANDLE hFile = CreateFile(L"captureqwsx.bmp",
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size
	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER);

	//Size of the file
	bmfHeader.bfSize = dwSizeofDIB;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM   

	DWORD dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	//Unlock and Free the DIB from the heap
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	//Close the handle for the file that was created
	CloseHandle(hFile);

	//Clean up
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
	ReleaseDC(hWnd, hdcWindow);

}
