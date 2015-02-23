#include "Win32_DX11AppUtil.h"

DirectX11 DX11;

bool DirectX11::IsAnyKeyPressed() const //checks if a key is set and sets key if true used for input messages
{
    for (unsigned i = 0; i < (sizeof(Key) / sizeof(Key[0])); i++)
        if (Key[i]) return true;
    return false;
}

void DirectX11::SetMaxFrameLatency(int value)
{
    IDXGIDevice1* DXGIDevice1 = NULL;//for refresh rate of monitor
    HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice1);//gets the interface of the device
    if (FAILED(hr) | (DXGIDevice1 == NULL)) return;//return if there was no device to display to; aka device removed
    DXGIDevice1->SetMaximumFrameLatency(value);
    DXGIDevice1->Release();// releases com object so it may be used by other functions
}

/* loops waiting for new messages from key press updates continuously*/
void DirectX11::WaitUntilGpuIdle()
{
    D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_EVENT, 0 };
    ID3D11Query *  query;
    BOOL           done = FALSE;
    if (Device->CreateQuery(&queryDesc, &query) == S_OK)
    {
        Context->End(query);
        while (!done && !FAILED(Context->GetData(query, &done, sizeof(BOOL), 0)));
    }
}

void DirectX11::HandleMessages()
{
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
}

void DirectX11::OutputFrameTime(double currentTime)
{
    static double lastTime = 0;
    char tempString[100];
    sprintf_s(tempString, "Frame time = %0.2f ms\n", (currentTime - lastTime)*1000.0f);
    OutputDebugStringA(tempString);
    lastTime = currentTime;
}

void DirectX11::ReleaseWindow(HINSTANCE hinst)
{
    DestroyWindow(DX11.Window); UnregisterClassW(L"OVRAppWindow", hinst);
};

//----------------------------------------------------------------------------------------------------------
void DirectX11::ClearAndSetRenderTarget(ID3D11RenderTargetView * rendertarget,
    ImageBuffer * depthbuffer, Recti vp)
{
    float black[] = { 0, 0, 0, 1 };
    Context->OMSetRenderTargets(1, &rendertarget, depthbuffer->TexDsv);
    Context->ClearRenderTargetView(rendertarget, black);
    Context->ClearDepthStencilView(depthbuffer->TexDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    D3D11_VIEWPORT D3Dvp;
    D3Dvp.Width = (float)vp.w;    D3Dvp.Height = (float)vp.h;
    D3Dvp.MinDepth = 0;              D3Dvp.MaxDepth = 1;
    D3Dvp.TopLeftX = (float)vp.x;    D3Dvp.TopLeftY = (float)vp.y;
    Context->RSSetViewports(1, &D3Dvp);
}

//---------------------------------------------------------------
LRESULT CALLBACK SystemWindowProc(HWND arg_hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case(WM_NCCREATE) : DX11.Window = arg_hwnd;                     break;
    case WM_KEYDOWN:    DX11.Key[(unsigned)wp] = true;              break;
    case WM_KEYUP:      DX11.Key[(unsigned)wp] = false;             break;
    }
    return DefWindowProc(DX11.Window, msg, wp, lp);
}

//-----------------------------------------------------------------------
bool DirectX11::InitWindowAndDevice(HINSTANCE hinst, Recti vp, bool windowed)
{
    WNDCLASSW wc; memset(&wc, 0, sizeof(wc));
    wc.lpszClassName = L"OVRAppWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = SystemWindowProc;
    wc.cbWndExtra = NULL;
    RegisterClassW(&wc);

    DWORD wsStyle = WS_POPUP;
    DWORD sizeDivisor = 1;

    if (windowed)
    {
        wsStyle |= WS_OVERLAPPEDWINDOW; sizeDivisor = 2;
    }

    RECT winSize = { 0, 0, vp.w / sizeDivisor, vp.h / sizeDivisor };
    AdjustWindowRect(&winSize, wsStyle, false);

    Window = CreateWindowW(L"OVRAppWindow", L"VR-Monitors", wsStyle | WS_VISIBLE,
        vp.x, vp.y, winSize.right - winSize.left, winSize.bottom - winSize.top,
        NULL, NULL, hinst, NULL);

    if (!Window)
        return(false);
    if (windowed)
        WinSize = vp.GetSize();
    else
    {
        RECT rc; GetClientRect(Window, &rc);
        WinSize = Sizei(rc.right - rc.left, rc.bottom - rc.top);
    }

    IDXGIFactory * DXGIFactory;
    IDXGIAdapter * Adapter;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&DXGIFactory))))
        return(false);
    if (FAILED(DXGIFactory->EnumAdapters(0, &Adapter)))
        return(false);
    if (FAILED(D3D11CreateDevice(Adapter, Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
        NULL, 0, NULL, 0, D3D11_SDK_VERSION, &Device, NULL, &Context)))
        return(false);

    DXGI_SWAP_CHAIN_DESC scDesc;
    memset(&scDesc, 0, sizeof(scDesc));
    scDesc.BufferCount = 2;
    scDesc.BufferDesc.Width = WinSize.w;
    scDesc.BufferDesc.Height = WinSize.h;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferDesc.RefreshRate.Numerator = 0;
    scDesc.BufferDesc.RefreshRate.Denominator = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = Window;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.Windowed = windowed;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

    if (FAILED(DXGIFactory->CreateSwapChain(Device, &scDesc, &SwapChain)))               return(false);
    if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer))) return(false);
    if (FAILED(Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRT)))         return(false);

    MainDepthBuffer = new ImageBuffer(true, true, Sizei(WinSize.w, WinSize.h));
    Context->OMSetRenderTargets(1, &BackBufferRT, MainDepthBuffer->TexDsv);
    if (!windowed) SwapChain->SetFullscreenState(1, NULL);
    UniformBufferGen = new DataBuffer(D3D11_BIND_CONSTANT_BUFFER, NULL, 2000);// make sure big enough

    D3D11_RASTERIZER_DESC rs;
    memset(&rs, 0, sizeof(rs));
    rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
    rs.CullMode = D3D11_CULL_NONE;
    rs.FillMode = D3D11_FILL_SOLID;
    ID3D11RasterizerState *  Rasterizer = NULL;
    Device->CreateRasterizerState(&rs, &Rasterizer);
    Context->RSSetState(Rasterizer);

    D3D11_DEPTH_STENCIL_DESC dss;
    memset(&dss, 0, sizeof(dss));
    dss.DepthEnable = true;
    dss.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ID3D11DepthStencilState * DepthState;
    Device->CreateDepthStencilState(&dss, &DepthState);
    Context->OMSetDepthStencilState(DepthState, 0);
    return(true);
}

//---------------------------------------------------------------------------------------------
void DirectX11::Render(ShaderFill* fill, DataBuffer* vertices, DataBuffer* indices, UINT stride, int count)
{
    Context->IASetInputLayout(fill->InputLayout);
    Context->IASetIndexBuffer(indices->D3DBuffer, DXGI_FORMAT_R16_UINT, 0);

    UINT offset = 0;
    Context->IASetVertexBuffers(0, 1, &vertices->D3DBuffer, &stride, &offset);
    UniformBufferGen->Refresh(fill->VShader->UniformData, fill->VShader->UniformsSize);
    Context->VSSetConstantBuffers(0, 1, &UniformBufferGen->D3DBuffer);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->VSSetShader(fill->VShader->D3DVert, NULL, 0);
    Context->PSSetShader(fill->PShader->D3DPix, NULL, 0);
    Context->PSSetSamplers(0, 1, &fill->SamplerState);
    if (fill->OneTexture)
        Context->PSSetShaderResources(0, 1, &fill->OneTexture->TexSv);
    Context->DrawIndexed(count, 0, 0);
}

//--------------------------------------------------------------------------------
// Due to be removed once the functionality is in the SDK
void UtilFoldExtraYawIntoTimewarpMatrix(Matrix4f * timewarpMatrix, Quatf eyePose, Quatf extraQuat)
{
    timewarpMatrix->M[0][1] = -timewarpMatrix->M[0][1];
    timewarpMatrix->M[0][2] = -timewarpMatrix->M[0][2];
    timewarpMatrix->M[1][0] = -timewarpMatrix->M[1][0];
    timewarpMatrix->M[2][0] = -timewarpMatrix->M[2][0];
    Quatf newtimewarpStartQuat = eyePose * extraQuat * (eyePose.Inverted())*(Quatf(*timewarpMatrix));
    *timewarpMatrix = Matrix4f(newtimewarpStartQuat);
    timewarpMatrix->M[0][1] = -timewarpMatrix->M[0][1];
    timewarpMatrix->M[0][2] = -timewarpMatrix->M[0][2];
    timewarpMatrix->M[1][0] = -timewarpMatrix->M[1][0];
    timewarpMatrix->M[2][0] = -timewarpMatrix->M[2][0];
}

/* again similar to the tutorial -- the id3d10blob is used which has a buffer associated with it
d3d10 is used because dx11 extends it and nothing new was added for dx11
@param which_type if 0 then vertex shader will be created else pixel shader
*/
Shader::Shader(ID3DBlob* s, int which_type) : numUniformInfo(0)
{
    if (which_type == 0) DX11.Device->CreateVertexShader(s->GetBufferPointer(), s->GetBufferSize(), NULL, &D3DVert);
    else               DX11.Device->CreatePixelShader(s->GetBufferPointer(), s->GetBufferSize(), NULL, &D3DPix);

    ID3D11ShaderReflection* ref;
    D3DReflect(s->GetBufferPointer(), s->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&ref);
    ID3D11ShaderReflectionConstantBuffer* buf = ref->GetConstantBufferByIndex(0);
    D3D11_SHADER_BUFFER_DESC bufd; //descriptior for the shader buffer
    if (FAILED(buf->GetDesc(&bufd))) return;

    for (unsigned i = 0; i < bufd.Variables; i++)
    {
        ID3D11ShaderReflectionVariable* var = buf->GetVariableByIndex(i);
        D3D11_SHADER_VARIABLE_DESC vd;//descriptor for the shader variable
        var->GetDesc(&vd);
        Uniform u;
        strcpy_s(u.Name, (const char*)vd.Name);;
        u.Offset = vd.StartOffset;
        u.Size = vd.Size;
        UniformInfo[numUniformInfo++] = u;
    }
    UniformsSize = bufd.Size;
    UniformData = (unsigned char*)OVR_ALLOC(bufd.Size);
}

void Shader::SetUniform(const char* name, int n, const float* v)
{
    for (int i = 0; i < numUniformInfo; i++)
    {
        if (!strcmp(UniformInfo[i].Name, name))
        {
            memcpy(UniformData + UniformInfo[i].Offset, v, n * sizeof(float));
            return;
        }
    }
}

ImageBuffer::ImageBuffer(bool rendertarget, bool depth, Sizei size, int mipLevels,
    unsigned char * data) : Size(size)
{
    D3D11_TEXTURE2D_DESC dsDesc;
    dsDesc.Width = size.w;
    dsDesc.Height = size.h;
    dsDesc.MipLevels = mipLevels;
    dsDesc.ArraySize = 1;
    dsDesc.Format = depth ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;//if depth was specified make format 32 bit color or else 
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.CPUAccessFlags = 0;
    dsDesc.MiscFlags = 0;
    dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    if (rendertarget &&  depth) dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (rendertarget && !depth) dsDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    DX11.Device->CreateTexture2D(&dsDesc, NULL, &Tex);
    DX11.Device->CreateShaderResourceView(Tex, NULL, &TexSv);

    if (rendertarget &&  depth) DX11.Device->CreateDepthStencilView(Tex, NULL, &TexDsv);
    if (rendertarget && !depth) DX11.Device->CreateRenderTargetView(Tex, NULL, &TexRtv);

    if (data) // Note data is trashed, as is width and height //not sure what this comment means i assume it is not very efficient
    {
        for (int level = 0; level < mipLevels; level++)
        {
            DX11.Context->UpdateSubresource(Tex, level, NULL, data, size.w * 4, size.h * 4);
            for (int j = 0; j < (size.h & ~1); j += 2)
            {
                const uint8_t* psrc = data + (size.w * j * 4);
                uint8_t*       pdest = data + ((size.w >> 1) * (j >> 1) * 4);
                for (int i = 0; i < size.w >> 1; i++, psrc += 8, pdest += 4)
                {
                    pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[size.w * 4 + 0] + psrc[size.w * 4 + 4]) >> 2;
                    pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[size.w * 4 + 1] + psrc[size.w * 4 + 5]) >> 2;
                    pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[size.w * 4 + 2] + psrc[size.w * 4 + 6]) >> 2;
                    pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[size.w * 4 + 3] + psrc[size.w * 4 + 7]) >> 2;
                }
            }
            size.w >>= 1;  size.h >>= 1;
        }
    }
}

ImageBuffer::ImageBuffer(bool rendertarget, bool depth, Sizei size, ID3D11Texture2D* newTex, ID3D11ShaderResourceView* newResource) : Size(size)
{

    Tex = newTex;

    TexSv = newResource;

    if (rendertarget &&  depth) DX11.Device->CreateDepthStencilView(Tex, NULL, &TexDsv);
    if (rendertarget && !depth) DX11.Device->CreateRenderTargetView(Tex, NULL, &TexRtv);

}


// Creates the shaders for the model

ShaderFill::ShaderFill(D3D11_INPUT_ELEMENT_DESC * VertexDesc, int numVertexDesc,
    int type, ImageBuffer * t, bool wrap)
    : OneTexture(t)
{
    ID3DBlob *vertextBlobData;
    ID3DBlob *pixelBlobData;
    ID3DBlob *errors;

    // 0 means it's a box
    switch (type) {
    case Box:
        D3DCompileFromFile(L"Source/VertexShaderBox.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &vertextBlobData, NULL);
        D3DCompileFromFile(L"Source/PixelShaderBox.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &pixelBlobData, NULL);
        break;
    case Skybox:
        D3DCompileFromFile(L"Source/VertexShaderSkybox.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &vertextBlobData, NULL);
        D3DCompileFromFile(L"Source/PixelShaderSkybox.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &pixelBlobData, NULL);
        break;
    case Monitor:
        D3DCompileFromFile(L"Source/VertexShaderMonitor.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &vertextBlobData, NULL);
        D3DCompileFromFile(L"Source/PixelShaderMonitor.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &pixelBlobData, &errors);
        break;
    default:
        break;
    }
    VShader = new Shader(vertextBlobData, 0);
    HRESULT hr;
    const void * gpb =  vertextBlobData->GetBufferPointer();
    SIZE_T s = vertextBlobData->GetBufferSize();
    hr = DX11.Device->CreateInputLayout(VertexDesc, numVertexDesc,
        vertextBlobData->GetBufferPointer(), vertextBlobData->GetBufferSize(), &InputLayout);
    PShader = new Shader(pixelBlobData, 1);

    D3D11_SAMPLER_DESC ss; memset(&ss, 0, sizeof(ss));
    ss.AddressU = ss.AddressV = ss.AddressW = wrap ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_BORDER;
    ss.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    ss.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    ss.MinLOD = 0;
    ss.MaxLOD = D3D11_FLOAT32_MAX;
    DX11.Device->CreateSamplerState(&ss, &SamplerState);
}

DataBuffer::DataBuffer(D3D11_BIND_FLAG use, const void* buffer, size_t size) : Size(size)
{
    D3D11_BUFFER_DESC desc;   memset(&desc, 0, sizeof(desc));
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.BindFlags = use;
    desc.ByteWidth = (unsigned)size;
    D3D11_SUBRESOURCE_DATA sr;
    sr.pSysMem = buffer;
    sr.SysMemPitch = sr.SysMemSlicePitch = 0;
    DX11.Device->CreateBuffer(&desc, buffer ? &sr : NULL, &D3DBuffer);
}

void DataBuffer::Refresh(const void* buffer, size_t size)
{
    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr = DX11.Context->Map(D3DBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    memcpy((void *)map.pData, buffer, size);
    DX11.Context->Unmap(D3DBuffer, 0);
}
