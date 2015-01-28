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
	case WM_SETFOCUS:   SetCapture(DX11.Window); ShowCursor(FALSE); break;
	case WM_KILLFOCUS:  ReleaseCapture(); ShowCursor(TRUE);         break;
	}
	return DefWindowProc(DX11.Window, msg, wp, lp);
}

//-----------------------------------------------------------------------
bool DirectX11::InitWindowAndDevice(HINSTANCE hinst, Recti vp, bool windowed)
{
	WNDCLASSW wc; memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"OVRAppWindow";
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
	Window = CreateWindowW(L"OVRAppWindow", L"OculusRoomTiny", wsStyle | WS_VISIBLE,
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
	rs.CullMode = D3D11_CULL_BACK;
	rs.FillMode = D3D11_FILL_SOLID;
	ID3D11RasterizerState *  Rasterizer = NULL;
	Device->CreateRasterizerState(&rs, &Rasterizer);
	Context->RSSetState(Rasterizer);

	D3D11_DEPTH_STENCIL_DESC dss;
	memset(&dss, 0, sizeof(dss));
	dss.DepthEnable = true;
	dss.DepthFunc = D3D11_COMPARISON_LESS;
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
