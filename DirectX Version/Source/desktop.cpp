#include "desktop.h"

Desktop::Desktop() : desktop(nullptr),
desktopImage(nullptr),
MetaDataBuffer(nullptr),
MetaDataSize(0),
OutputNumber(0),
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

    hr = desktop->AcquireNextFrame(10, &frameData, &desktopResource);
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

    // QI for IDXGIResource
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&desktopImage));
    data->Frame = desktopImage;

    desktopResource->Release();
    desktopResource = nullptr;
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

void Desktop::init() {
    HDESK CurrentDesktop = nullptr;
    UINT Output = 0;
    HRESULT hr;
    CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
    if (!CurrentDesktop)
    {
        // We do not have access to the desktop so request a retry
    }

    // Attach desktop to this thread
    bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
    CloseDesktop(CurrentDesktop);
    CurrentDesktop = nullptr;
    if (!DesktopAttached)
    {
        // We do not have access to the desktop so request a retry
    }

    // Get DXGI device
    IDXGIDevice* DxgiDevice = nullptr;
    hr = Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
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
}