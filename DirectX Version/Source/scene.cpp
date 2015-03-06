#include "scene.h"
#include "desktop.h"
#include "../3rdParty/ScreenGrab/ScreenGrab.h"


D3D11_INPUT_ELEMENT_DESC ModelVertexDesc[] =
{ { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Model::Vertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Model::Vertex, C), D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Model::Vertex, U), D3D11_INPUT_PER_VERTEX_DATA, 0 }, };
D3D11_INPUT_ELEMENT_DESC ModelVertexDescMon[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "Color", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const float PI = 3.1415927f;

//used in addmonitor and initialization just below here for the first "screen"
//startFloat startingPoint(-0.5, -0.5, 0.8, 0.5, 0.5, 0.8, Model::Color(128, 128, 128));

void Scene::Add(Model * n)
{
    Models[num_models++] = n;
}

// Main world
Scene::Scene() :
num_models(0), num_monitors(0), monitorHeight(2), monitorWidth(2), monitorDepth(2), doRender(true),
startingPoint(-.5, -.5, -.5, .5, .5, .5,//monitorWidth, monitorHeight, monitorDepth,
Model::Color(128, 128, 128))
{
    Vector3f monitorOffset = Vector3f(0, 0, 0);

    // Construct textures
    static Model::Color tex_pixels[4][256 * 256];

    // this is what makes the checkered floors and walls textures 
    for (int k = 0; k < 4; k++)
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

    //create 4  capture threads
    for (int i = 0; i < 4; i++) {
        num_monitors++;//add one to monitors here to determine how to group them below
        Desktop*  d = new Desktop(num_monitors - 1); //num_monitors is 1 indexed instead of 0

        // add new monitor
        Model* m = new Model(Vector3f(i - 2 , (i*.1), startingPoint.z1), generated_texture[0]);
        m->active = false;
        m->Output = num_monitors - 1;
        m->desktop = d;
        //ResumeThread(m->thread);//start thread
        // add new desktop
        //d->init(false, true);

        //everything is added based on the first monitor the startingpoint monitor
        m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
            startingPoint.y2, startingPoint.z2, startingPoint.color);
        m->AllocateBuffers();
        Add(m);//add monitor to scene array to be rendered;
        Monitors[num_monitors - 1] = m;
    }
    //start the default
    DWORD threadID;
    Monitors[0]->thread = CreateThread(nullptr, 0, captureDesktop, Monitors[0], CREATE_SUSPENDED, &threadID);
    ResumeThread(Monitors[0]->thread);
    activeDesktop = 0;

    // skybox
    Model* m = new Model(Vector3f(0, 0, 0), generated_texture[4]);
    m->AddSolidColorBox(-10, -10, -10, 10, 10, 10, Model::Color(128, 128, 128));
    m->AllocateBuffers();
    m->Pos.z = -3;
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
    return Monitors[num_monitors - 1]->Pos;
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

DWORD WINAPI captureDesktop(void* params) {
    Model* input = reinterpret_cast<Model*>(params);
    input->desktop->init(input->Output);
    D3D11_TEXTURE2D_DESC frameDesc;
    RtlZeroMemory(&frameDesc, sizeof(D3D11_TEXTURE2D_DESC));
    input->desktop->stage->GetDesc(&frameDesc);
    HRESULT hr;
    if (!input->desktop->masterImage) {
        D3D11_TEXTURE2D_DESC dsDesc2;
        RtlZeroMemory(&dsDesc2, sizeof(D3D11_TEXTURE2D_DESC));
        dsDesc2.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        dsDesc2.CPUAccessFlags = 0;
        dsDesc2.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        dsDesc2.Width = frameDesc.Width;
        dsDesc2.Height = frameDesc.Height;
        dsDesc2.MipLevels = frameDesc.MipLevels;
        dsDesc2.ArraySize = 1;
        dsDesc2.Format = frameDesc.Format;
        dsDesc2.SampleDesc.Count = 1;
        dsDesc2.SampleDesc.Quality = 0;
        dsDesc2.Usage = D3D11_USAGE_DEFAULT;
        hr = DX11.Device->CreateTexture2D(&dsDesc2, nullptr, &(input->desktop->masterImage));
    }
    HANDLE Hnd(NULL);
    IDXGIResource* DXGIResource = nullptr;
    hr = input->desktop->masterImage->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&DXGIResource));
    DXGIResource->GetSharedHandle(&Hnd);
    DXGIResource->Release();
    DXGIResource = nullptr;
    hr = DX11.Device->OpenSharedResource(Hnd, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&(input->desktop->stageHandle)));

    DX11.Device->CreateShaderResourceView(input->desktop->masterImage, NULL, &(input->desktop->masterView));
    input->desktop->masterBuffer = new ImageBuffer(true, false, Sizei(frameDesc.Width, frameDesc.Height),
        input->desktop->masterImage, input->desktop->masterView);
    input->desktop->masterFill = new ShaderFill(ModelVertexDescMon, 3, 0, input->desktop->masterBuffer);
    input->Fill = input->desktop->masterFill; //there probably isn't a point keeping a pointer in the desktop if the model is being passed around
    //generate a mutex for the master image
    IDXGIKeyedMutex* KeyMutex = nullptr;
    hr = input->desktop->stage->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&KeyMutex));
    input->active = true;
    bool skipCapture = false;
    FRAME_DATA frame;
    bool timedout;
    while (input->active) {
        //capture
        Sleep(300); //sleep for 30 ms gives us about 30 fps
       // hr = KeyMutex->AcquireSync(0, 100); //possibly dangerous because an old image will be renderd if the mutex isn't released 
        if (hr == WAIT_TIMEOUT) {
            //we don't have access try again later
            //techincally this should never happen
            //input->desktop->relaseFrame();
            skipCapture = true;
            continue;
        }

        //release to be safe
        input->desktop->relaseFrame();
        input->desktop->getFrame(&frame, &timedout);
        if (frame.Frame == nullptr || timedout) {
            //could not get frame try again later
            input->desktop->relaseFrame();
            hr = KeyMutex->ReleaseSync(1);
            continue;
        }

        // the frame that comes from the desktop duplication api is not sharable so we must copy it to a controled resource
        skipCapture = false;
        HRESULT hr;
        input->desktop->deviceContext->CopyResource(input->desktop->stage, frame.Frame);
        input->desktop->deviceContext->CopyResource(input->desktop->stageHandle, input->desktop->stage);

        SaveDDSTextureToFile(input->desktop->deviceContext, input->desktop->stage, L"stage.dds");
        SaveDDSTextureToFile(input->desktop->deviceContext, frame.Frame, L"frame.dds");
        SaveDDSTextureToFile(DX11.Context, input->desktop->masterImage, L"masterImage.dds");
        //SaveDDSTextureToFile(input->desktop->deviceContext, input->desktop->stageHandle, L"stageHandle.dds");
        
        // SaveDDSTextureToFile(input->desktop->deviceContext, input->desktop->stage, L"stage.dds");
        input->desktop->relaseFrame();
       // hr = KeyMutex->ReleaseSync(1);
    }
Exit:
    //do cleanup work
    return 0;

}

void Scene::addMonitor(float yaw, Vector3f _pos){ //we probably won't need this anymore
    //deactivate all monitors
    for (int i = 0; i < num_monitors; i++) {
        Monitors[i]->active = false;
        SuspendThread(Monitors[i]->thread);
    }
    if (num_monitors < 3) {
        static Model::Color tex_pixels[4][256 * 256];
        //we would probably fill this tex with pixels from desktop dup here
        num_monitors++;//add one to monitors here to determine how to group them below
        Desktop*  d = new Desktop(num_monitors - 1); //num_monitors is 1 indexed instead of 0

        //spawn a new thread

        DWORD threadID;
        // add new monitor
        Model* m = new Model(Vector3f(0, 0, startingPoint.z1), generated_texture[0]);
        m->Output = num_monitors - 1;
        m->desktop = d;

        m->thread = CreateThread(nullptr, 0, captureDesktop, m, 0, &threadID);

        _pos = Vector3f(0, 0, 0);

        //everything is added based on the first monitor the startingpoint monitor
        m->AddSolidColorBox(startingPoint.x1, startingPoint.y1, startingPoint.z1, startingPoint.x2,
            startingPoint.y2, startingPoint.z2, startingPoint.color);
        m->AllocateBuffers();
        Add(m);//add monitor to scene array to be rendered;
        Monitors[num_monitors - 1] = m;//add this monitor to the array of monitors	//if we have two monitors on the bottom then we need to add some up top
        //so get where we started from and add an offset, only supports about 5 monitors total right now
        //Vector3f temp = getLastMonitorPosition();
        //Vector3f tempVect = getOffset() + getLastMonitorPosition();//initialize in case we change in loop below
        if (num_monitors == 2){//change position of first monitor
            _pos = Vector3f(-.9, 0, -.9);
            Monitors[1]->Pos = _pos;
            Monitors[1]->OriginalPos = _pos;
            //Monitors[0]->Pos.x = ;//do i need to shift the monitor position?
            //Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw - PI / 6.5);
            Monitors[1]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI / 6.5);
            Monitors[1]->OriginalRot = Monitors[1]->Rot;
            _pos = Vector3f(.9, 0, 0);
            Monitors[0]->Pos = _pos;
            Monitors[0]->OriginalPos = _pos;
            //Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), -PI + yaw + PI / 6.5);
            Monitors[0]->Rot = Quatf(Vector3f(0, _pos.y == 0 ? .001 : _pos.y, 0), +PI / 6.5);
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

