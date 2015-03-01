#include "desktop.h"
//forward declaration
D3D11_INPUT_ELEMENT_DESC ModelVertexDescMon[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "Color", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

Desktop::Desktop() : desktop(nullptr),
desktopImage(nullptr),
metaDataBuffer(nullptr),
metaDataSize(0),
Device(nullptr)
{
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
    if (FAILED(hr))
    {
        //could not create
    }

}

int Desktop::getFrame(FRAME_DATA* data, bool* timedout) {
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameData;
    HRESULT hr;

    hr = desktop->AcquireNextFrame(1, &frameData, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        *timedout = true;
        return -1;
    }
    *timedout = false;

    if (FAILED(hr)) {
        //could not get frame
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


    //get metadata for dirty count
    // Get metadata
    if (frameData.TotalMetadataBufferSize)
    {
        // Old buffer too small
        if (frameData.TotalMetadataBufferSize > metaDataSize)
        {
            if (metaDataBuffer) // wipe out old buffer
            {
                delete[] metaDataBuffer;
                metaDataBuffer = nullptr;
            }
            metaDataBuffer = new (std::nothrow) BYTE[frameData.TotalMetadataBufferSize];
            if (!metaDataBuffer)
            {
                metaDataBuffer = 0;
                data->MoveCount = 0;
                data->DirtyCount = 0;
                return -1; //failed to allocate buffer
            }
            metaDataSize = frameData.TotalMetadataBufferSize;
        }

        UINT BufSize = frameData.TotalMetadataBufferSize;

        // Get move rectangles
        hr = desktop->GetFrameMoveRects(BufSize, reinterpret_cast<DXGI_OUTDUPL_MOVE_RECT*>(metaDataBuffer), &BufSize);
        if (FAILED(hr))
        {
            data->MoveCount = 0;
            data->DirtyCount = 0;
            return -1;
        }
        data->MoveCount = BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);

        BYTE* DirtyRects = metaDataBuffer + BufSize;
        BufSize = frameData.TotalMetadataBufferSize - BufSize;

        // Get dirty rectangles
        hr = desktop->GetFrameDirtyRects(BufSize, reinterpret_cast<RECT*>(DirtyRects), &BufSize);
        if (FAILED(hr))
        {
            data->MoveCount = 0;
            data->DirtyCount = 0;
            return -1; //failed to get dirty rects
        }
        data->DirtyCount = BufSize / sizeof(RECT);
        data->MetaData = metaDataBuffer;
    }

    return 0;
}

int Desktop::relaseFrame(){
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


void Desktop::init(boolean newMonitor, boolean newDesktop) {

	HDESK mainDesktop = GetThreadDesktop(GetCurrentThreadId());

	HDESK CurrentDesktop = nullptr;
	thread = nullptr;

	if (newDesktop) {
		/*
		_THREAD_DATA * threadData = new _THREAD_DATA;
		threadData->OffsetX = 1920;
		threadData->OffsetY = 1080;
		threadData->PtrInfo = &ptrInfo;
		*/

		CurrentDesktop = CreateDesktop(TEXT("Virtual Desktop"), NULL, NULL, DF_ALLOWOTHERACCOUNTHOOK, GENERIC_ALL, NULL);
		//thread = CreateThread(NULL, 0, NULL, &threadData, 0, NULL);

		system("start explorer");
		WCHAR cmd[] = L"explorer";
		STARTUPINFOW si = { 0 };
		si.cb = sizeof (si);
		si.lpDesktop = L"Virtual Desktop";
		si.wShowWindow = SW_SHOW;
		PROCESS_INFORMATION pi;
		CreateProcessW(NULL, cmd, 0, 0, FALSE, NULL, NULL, NULL, &si, &pi);

		SwitchDesktop(CurrentDesktop);
		SetThreadDesktop(CurrentDesktop);
	}


	UINT Output = 0;

    HRESULT hr;


    // Get DXGI device
    IDXGIDevice* DxgiDevice = nullptr;
    hr = DX11.Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
    if (FAILED(hr))
    {
    }

    // Get DXGI adapter
    IDXGIAdapter* DxgiAdapter = nullptr;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
    DxgiDevice->Release();
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
    }

	if (newDesktop){
		SwitchDesktop(mainDesktop);
		SetThreadDesktop(mainDesktop);
	}

	if (newMonitor) {
		Output = 1;
	}

    // Get output
    IDXGIOutput* DxgiOutput = nullptr;
    hr = DxgiAdapter->EnumOutputs(Output, &DxgiOutput);
    DxgiAdapter->Release();
    DxgiAdapter = nullptr;
    if (FAILED(hr))
    {
    }

    DxgiOutput->GetDesc(&OutputDesc);


    // QI for Output 1
    IDXGIOutput1* DxgiOutput1 = nullptr;
    hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
    DxgiOutput->Release();
    DxgiOutput = nullptr;
    if (FAILED(hr))
    {
    }

    hr = DxgiOutput1->DuplicateOutput(Device, &desktop);
    DxgiOutput1->Release();
    DxgiOutput1 = nullptr;

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

    hr = DX11.Device->CreateTexture2D(&dsDesc2, nullptr, &masterImage);
    
    DX11.Context->CopyResource(masterImage, tmp);
    DX11.Device->CreateShaderResourceView(masterImage, NULL, &masterView);
    masterBuffer = new ImageBuffer(true, false, Sizei(frameDesc.Width, frameDesc.Height), masterImage, masterView);
    masterFill = new ShaderFill(ModelVertexDescMon, 3, 0, masterBuffer);
    this->relaseFrame();

}