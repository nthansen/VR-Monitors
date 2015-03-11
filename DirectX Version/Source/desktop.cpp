#include "desktop.h"
#include "../3rdParty/ScreenGrab/ScreenGrab.h"

//forward declaration
D3D11_INPUT_ELEMENT_DESC ModelVertexDescMon[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "Color", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
Desktop::Desktop(int id) : Desktop() {
    outputNumber = id;

    switch (id) {
    case 0:
        //the is the original 'workspace'
        //establish primary desktop for the first output
        // named Default
        initialized = true;
        desktopName = L"Default";
        mainDesktop = OpenDesktop(L"Default", DF_ALLOWOTHERACCOUNTHOOK, true, DESKTOP_JOURNALRECORD | DESKTOP_JOURNALPLAYBACK | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | DESKTOP_SWITCHDESKTOP | DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_READOBJECTS);
        break;
    case 1:
        desktopName = L"Sysinternals Desktop 1";
        //create to double check that the desktop exist
        mainDesktop = CreateDesktop(L"Sysinternals Desktop 1", NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    case 2:
        desktopName = L"Sysinternals Desktop 2";
        mainDesktop = CreateDesktop(L"Sysinternals Desktop 2", NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    case 3:
        desktopName = L"Sysinternals Desktop 3";
        mainDesktop = CreateDesktop(L"Sysinternals Desktop 3", NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    }
}
Desktop::Desktop() : desktop(nullptr),
desktopImage(nullptr),
pointerImage(nullptr),
masterImage(nullptr),
stage(nullptr),
metaDataBuffer(nullptr),
masterView(nullptr),
metaDataSize(0),
Device(nullptr),
initialized(false)
{
    //empty out pointer informmation
    pointer.BufferSize = 0;
    pointer.Position.x = 0;
    pointer.Position.y = 0;
    pointer.PtrShapeBuffer = nullptr;
}

int Desktop::getFrame(FRAME_DATA* data, bool* timedout) {
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameData;
    HRESULT hr;

    hr = desktop->AcquireNextFrame(0, &frameData, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        *timedout = true;
        return -1;
    }
    *timedout = false;

    if (FAILED(hr)) {
        //could not get frame
        if (hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_INVALID_CALL) {
            //reinitialize
            //check if current desktop
            HDESK currentDesktop = OpenInputDesktop(0, false, DESKTOP_READOBJECTS);
            wchar_t data[100]; //name should never be more than 100
            bool result = GetUserObjectInformation(currentDesktop, UOI_NAME, data, (100 * sizeof(char)), nullptr);
            if (wcscmp((LPWSTR)(data), desktopName) == 0) {
                Sleep(100); //for some reason if the desktops are switch too quickly we won't have access to it
                desktop->Release();
                desktop = nullptr;
                init(output);
                *timedout = true;
                CloseDesktop(currentDesktop);
                return -1;
            }
            else {
                CloseDesktop(currentDesktop);
            }
        }

        *timedout = true;
        return -1;
    }

    if (desktopImage){
        //release old frame
        desktopImage->Release();
        desktopImage = nullptr;
    }
    if (frameData.LastPresentTime.QuadPart == 0) { //frame has not been updated since the last pull, set to nulll
        data->Frame = nullptr;
    }
    else {
        // QI for IDXGIResource
        hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&desktopImage));
        data->Frame = desktopImage;
    }

    pointer.Visible = frameData.PointerPosition.Visible;
    if (frameData.LastMouseUpdateTime.QuadPart != 0 && frameData.PointerPosition.Visible && 
		frameData.PointerPosition.Position.x >= 0 && frameData.PointerPosition.Position.y >= 0) { //there is a mouse on the screen capture the buffer
        if (frameData.PointerShapeBufferSize != 0) {
            //new shape update the buffer
            if (pointer.BufferSize != frameData.PointerShapeBufferSize) {
                pointer.BufferSize = frameData.PointerShapeBufferSize;
                if (pointer.PtrShapeBuffer) {
                    //release old buffer
                    delete[] pointer.PtrShapeBuffer;
                    pointer.PtrShapeBuffer = nullptr;
                }
                //allocate new buffer space
                pointer.PtrShapeBuffer = new (std::nothrow) BYTE[pointer.BufferSize]; //mouse is not critical, don't throw an error
                if (pointer.PtrShapeBuffer == nullptr) {
                    //could not allocate
                    pointer.BufferSize = 0;
                }
            }
            //get the shape
            UINT requiredBufferSize;
            hr = desktop->GetFramePointerShape(pointer.BufferSize, reinterpret_cast<VOID*>(pointer.PtrShapeBuffer), &requiredBufferSize, &(pointer.ShapeInfo));
            //create texture2d
            if (pointerImage){
                //release old image
                pointerImage->Release();
                pointerImage = nullptr;
            }
            D3D11_TEXTURE2D_DESC Desc;
            Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
            Desc.MipLevels = 1;
            Desc.ArraySize = 1;
            Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            Desc.SampleDesc.Count = 1;
            Desc.SampleDesc.Quality = 0;
            Desc.Usage = D3D11_USAGE_DEFAULT;
            Desc.CPUAccessFlags = 0;

            Desc.Width = pointer.ShapeInfo.Width;
            Desc.Height = pointer.ShapeInfo.Height;
            D3D11_SUBRESOURCE_DATA ptrData;
            ptrData.pSysMem = pointer.PtrShapeBuffer;
            ptrData.SysMemPitch = pointer.ShapeInfo.Pitch;
            ptrData.SysMemSlicePitch = 0;
            Device->CreateTexture2D(&Desc, &ptrData, &pointerImage);
        }
		if (pointerImage == NULL) {
			return 0;
		}
        //update the position of the mouse
        //there might need to be an offset for a multiple monitor setup
        pointer.Position.x = frameData.PointerPosition.Position.x;
        pointer.Position.y = frameData.PointerPosition.Position.y;
        pointer.LastTimeStamp = frameData.LastMouseUpdateTime;

        /*
        */
        D3D11_TEXTURE2D_DESC Desc;
        ZeroMemory(&Desc, sizeof(Desc));

        Desc.MiscFlags = 0;
        Desc.MipLevels = 1;
        Desc.ArraySize = 1;
        Desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.SampleDesc.Quality = 0;
        Desc.Usage = D3D11_USAGE_STAGING;
        Desc.BindFlags = 0;
        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        Desc.Width = pointer.ShapeInfo.Width;
        Desc.Height = pointer.ShapeInfo.Height;
        if (pointerImage) {
            pointerImage->Release();
            pointerImage = nullptr;
        }
        Device->CreateTexture2D(&Desc, NULL, &pointerImage);
        D3D11_BOX location;
        location.left = pointer.Position.x;
        location.top = pointer.Position.y;
        location.front = 0;
        location.right = pointer.Position.x + pointer.ShapeInfo.Width;
        location.bottom = pointer.Position.y + pointer.ShapeInfo.Height;
        location.back = 1;
        deviceContext->CopySubresourceRegion(
            pointerImage,
            0,
            0,
            0,
            0,
            data->Frame,
            0,
            &location);
        // SaveDDSTextureToFile(deviceContext, pointerImage, L"pointer1.dds");

        //map data
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        //Map the resources. Blocks the GPU from accessing the file. 
        hr = deviceContext->Map(pointerImage, 0, D3D11_MAP_READ_WRITE, 0, &mappedResource);
        if (!mappedResource.pData) {
            //unmap
            deviceContext->Unmap(pointerImage, 0);
        }
        else {
            UCHAR* pTexels = (UCHAR*)mappedResource.pData;
            for (int i = 0; i < pointer.BufferSize; i += 4) {
                //get rgb values
                int r = (UINT)pointer.PtrShapeBuffer[i];
                int g = (UINT)pointer.PtrShapeBuffer[i + 1];
                int b = (UINT)pointer.PtrShapeBuffer[i + 2];
                int a = (UINT)pointer.PtrShapeBuffer[i + 3];
                if (r == 0 && g == 0 && b == 0 && a == 0) {
                    continue;
                }
                pTexels[i] = r;
                pTexels[i + 1] = g;
                pTexels[i + 2] = b;
                pTexels[i + 3] = a;
            }
            deviceContext->Unmap(pointerImage, 0);
        }
        //loop through image
    }

    return 0;
}

int Desktop::relaseFrame(){
    if (!desktop){
        return -1;
    }
    HRESULT hr = desktop->ReleaseFrame();
    if (FAILED(hr))
    {
        return -1;
    }

    if (desktopImage)
    {
        desktopImage->Release();
        desktopImage = nullptr;
    }
    return 0;
}

BOOL __stdcall EnumDesktopProc(LPTSTR lpszDesktop, LPARAM lParam){
    return true;
}

void Desktop::newDesktop(int id) {
    /*
    _THREAD_DATA * threadData = new _THREAD_DATA;
    threadData->OffsetX = 1920;
    threadData->OffsetY = 1080;
    threadData->PtrInfo = &ptrInfo;
    */
    mainDesktop = GetThreadDesktop(GetCurrentThreadId());
    EnumDesktops(NULL, EnumDesktopProc, NULL);
    HDESK targetDesk;
    LPWSTR targetName;

    switch (id) {
    default:
        //the is the original 'workspace'
        //establish primary desktop for the first output
        // named Default
        targetName = L"Default";
        targetDesk = OpenDesktop(L"Default", DF_ALLOWOTHERACCOUNTHOOK, true, DESKTOP_JOURNALRECORD | DESKTOP_JOURNALPLAYBACK | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | DESKTOP_SWITCHDESKTOP | DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_READOBJECTS);
        break;
    case 1:
        //create to double check that the desktop exist
        targetName = L"Sysinternals Desktop 1";
        targetDesk = CreateDesktop(targetName, NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    case 2:
        targetName = L"Sysinternals Desktop 2";
        targetDesk = CreateDesktop(targetName, NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    case 3:
        targetName = L"Sysinternals Desktop 3";
        targetDesk = CreateDesktop(targetName, NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
        break;
    }
    //thread = CreateThread(NULL, 0, NULL, &threadData, 0, NULL);

    //system("start explorer");
    WCHAR cmd[] = L"VR-Monitors.exe";
    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    si.lpDesktop = targetName;
    si.wShowWindow = SW_SHOW;
    PROCESS_INFORMATION pi;
    wchar_t exepath[MAX_PATH + 100]; // enough room for cwd and the rest of command line
    GetCurrentDirectory(MAX_PATH, exepath);
    int result = CreateProcess(NULL, cmd, 0, 0, FALSE, NULL, NULL, NULL, &si, &pi);
    if (result == 0) {
        DWORD lastError = GetLastError();
        printf("placeholder");
    }
    SwitchDesktop(targetDesk);
    //SetThreadDesktop(targetDesk);

}

//Starts the capture of the desktop object
void Desktop::init(int outputNumber) {
    //EnumWindowStations(EnumDesktopProc, NULL);
    //EnumDesktops(GetProcessWindowStation(), EnumDesktopProc, NULL);
    //switch the desktop to the current desktop 
    output = outputNumber;/*
    SwitchDesktop(mainDesktop);
    SetThreadDesktop(mainDesktop)*/;
    DWORD error = GetLastError();
    if (!initialized) {
        /*
        _THREAD_DATA * threadData = new _THREAD_DATA;
        threadData->OffsetX = 1920;
        threadData->OffsetY = 1080;
        threadData->PtrInfo = &ptrInfo;
        */ //thread = CreateThread(NULL, 0, NULL, &threadData, 0, NULL);

        //SwitchDesktop(CurrentDesktop);

        //  system("start explorer");
        /*
        WCHAR cmd[] = L"explorer.exe";
        STARTUPINFOW si = { 0 };
        si.cb = sizeof(si);
        si.lpDesktop = desktopName;
        si.wShowWindow = SW_SHOW;
        PROCESS_INFORMATION pi;
        CreateProcessW(NULL, cmd, 0, 0, FALSE, NULL, NULL, NULL, &si, &pi);*/
        initialized = true;
    }
    //initilize the desktpp
    RtlZeroMemory(&OutputDesc, sizeof(OutputDesc));
    HRESULT hr;
    D3D_DRIVER_TYPE drivers[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };
    UINT featureLength = ARRAYSIZE(FeatureLevels);
    D3D_FEATURE_LEVEL FeatureLevel;

    for (UINT i = 0; i < 3; i++) {
        //create the dxgi adapter
        //driver type is not software so it's ok to pass null for the HMODULE   
        hr = D3D11CreateDevice(nullptr, drivers[i], nullptr, 0, FeatureLevels,
            featureLength, D3D11_SDK_VERSION, &Device, &FeatureLevel, &deviceContext);
        if (SUCCEEDED(hr))
        {
            break;
        }
    }

    if (!desktop) {
        // Get DXGI device
        IDXGIDevice* DxgiDevice = nullptr;
        hr = DX11.Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
        if (FAILED(hr))
        {
            printf("failed");
        }

        // Get DXGI adapter
        IDXGIAdapter* DxgiAdapter = nullptr;
        hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
        DxgiDevice->Release();
        DxgiDevice = nullptr;
        if (FAILED(hr))
        {
            printf("failed");
        }

        // Get output
        IDXGIOutput* DxgiOutput = nullptr;
        hr = DxgiAdapter->EnumOutputs(output, &DxgiOutput);
        DxgiAdapter->Release();
        DxgiAdapter = nullptr;
        if (FAILED(hr))
        {
            printf("failed");
        }

        DxgiOutput->GetDesc(&OutputDesc);


        // QI for Output 1
        IDXGIOutput1* DxgiOutput1 = nullptr;
        hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
        DxgiOutput->Release();
        DxgiOutput = nullptr;
        if (FAILED(hr))
        {
            printf("failed");
        }

        hr = DxgiOutput1->DuplicateOutput(Device, &desktop);
        DxgiOutput1->Release();
        DxgiOutput1 = nullptr;
        if (FAILED(hr))
        {
            printf("failed");
            //shut down program
            return;
        }
    }
    FRAME_DATA data;
    bool timedout = true;
    //capture the frame to get the full description of the output
    do{
        this->relaseFrame();
        this->getFrame(&data, &timedout);
    } while (timedout || data.Frame == NULL);
    D3D11_TEXTURE2D_DESC frameDesc;
    data.Frame->GetDesc(&frameDesc);

    D3D11_TEXTURE2D_DESC dsDesc;

    RtlZeroMemory(&dsDesc, sizeof(D3D11_TEXTURE2D_DESC));
    dsDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    dsDesc.CPUAccessFlags = 0;
    dsDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
    dsDesc.Width = frameDesc.Width;
    dsDesc.Height = frameDesc.Height;
    dsDesc.MipLevels = frameDesc.MipLevels;
    dsDesc.ArraySize = 1;
    dsDesc.Format = frameDesc.Format;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = Device->CreateTexture2D(&dsDesc, nullptr, &stage);
    deviceContext->CopyResource(stage, data.Frame);  //we save the data to an intermediary 

    D3D11_TEXTURE2D_DESC dsDesc2;

    RtlZeroMemory(&dsDesc, sizeof(D3D11_TEXTURE2D_DESC));
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

    HANDLE Hnd(NULL);
    IDXGIResource* DXGIResource = nullptr;
    hr = stage->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&DXGIResource));
    DXGIResource->GetSharedHandle(&Hnd);
    DXGIResource->Release();
    DXGIResource = nullptr;
    ID3D11Texture2D* tmp;
    hr = DX11.Device->OpenSharedResource(Hnd, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&tmp));

    if (!masterImage) {
        hr = DX11.Device->CreateTexture2D(&dsDesc2, nullptr, &masterImage);
    }
    DX11.Context->CopyResource(masterImage, tmp);
    if (!masterView) {
        DX11.Device->CreateShaderResourceView(masterImage, NULL, &masterView);
        masterBuffer = new ImageBuffer(true, false, Sizei(frameDesc.Width, frameDesc.Height), masterImage, masterView);
        masterFill = new ShaderFill(ModelVertexDescMon, 3, 0, masterBuffer);
    }
    this->relaseFrame();
}