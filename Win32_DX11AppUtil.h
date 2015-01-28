/************************************************************************************
Filename    :   Win32_DX11AppUtil.h
Content     :   D3D11 and Application/Window setup functionality for RoomTiny
Created     :   October 20th, 2014
Author      :   Tom Heath
Copyright   :   Copyright 2014 Oculus, Inc. All Rights reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
http://www.apache.org/licenses/LICENSE-2.0
 
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/
#ifndef WIN32_DX11APPUTIL
#define WIN32_DX11APPUTIL 

#include "Kernel/OVR_Math.h"
#include <d3d11.h>
#include <d3dcompiler.h>

using namespace OVR;

//---------------------------------------------------------------------
struct DirectX11
{
    HWND                     Window; //handle for the window
    bool                     Key[256]; // basically an array for each key sets true when set
    Sizei                    WinSize;
    struct ImageBuffer     * MainDepthBuffer;
    ID3D11Device           * Device;
    ID3D11DeviceContext    * Context;
    IDXGISwapChain         * SwapChain;
    ID3D11Texture2D        * BackBuffer;
    ID3D11RenderTargetView * BackBufferRT;
    struct DataBuffer      * UniformBufferGen;

    bool InitWindowAndDevice(HINSTANCE hinst, Recti vp,  bool windowed);
    void ClearAndSetRenderTarget(ID3D11RenderTargetView * rendertarget, ImageBuffer * depthbuffer, Recti vp);
    void Render(struct ShaderFill* fill, DataBuffer* vertices, DataBuffer* indices,UINT stride, int count);

	bool IsAnyKeyPressed() const; //checks if a key is set and sets key if true used for input messages

	//sets the number of frames the system is allowed to queue for rendering
	//needs to access the gpu to get the correct settings of the device
	//if max is not set then performance will suffer and vsync will be off
	//get numerator and denominator of refresh rate of monitor
	void SetMaxFrameLatency(int value);

	/* loops waiting for new messages from key press updates continuously*/
	void WaitUntilGpuIdle();

	//note uses peekmessage as in directx tutorial this optimizes performance by not entering new loop while messages are not found
	void HandleMessages();

	/*monitors frame rate*/
	void OutputFrameTime(double currentTime);

	/* if the window was closed this will close all windows related to this window handle*/
	void ReleaseWindow(HINSTANCE hinst);

};

extern DirectX11 DX11;

//--------------------------------------------------------------------------

/*
Shaders are used to drive the rendering pipleline. Each stage has a shader associated with it.
this struct uses two which is similar to the dxtutorial we completed -- a vertex shader and pixelshader
for more information check required reading final lesson tutorial
*/
struct Shader 
{
    ID3D11VertexShader * D3DVert;
    ID3D11PixelShader  * D3DPix;
    unsigned char      * UniformData;
    int                  UniformsSize;

    struct Uniform  {
        char Name[40];
        int Offset, Size;
    };

    int                  numUniformInfo;
    Uniform              UniformInfo[10];
 

	/* again similar to the tutorial -- the id3d10blob is used which has a buffer associated with it
	d3d10 is used because dx11 extends it and nothing new was added for dx11
	@param which_type if 0 then vertex shader will be created else pixel shader
	*/
    Shader(ID3D10Blob* s, int which_type) : numUniformInfo(0)
    {
        if (which_type==0) DX11.Device->CreateVertexShader(s->GetBufferPointer(),s->GetBufferSize(), NULL, &D3DVert);
        else               DX11.Device->CreatePixelShader(s->GetBufferPointer(),s->GetBufferSize(), NULL, &D3DPix);
 
        ID3D11ShaderReflection* ref;
        D3DReflect(s->GetBufferPointer(), s->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &ref);
        ID3D11ShaderReflectionConstantBuffer* buf = ref->GetConstantBufferByIndex(0);
        D3D11_SHADER_BUFFER_DESC bufd; //descriptior for the shader buffer
        if (FAILED(buf->GetDesc(&bufd))) return;
     
        for(unsigned i = 0; i < bufd.Variables; i++)
        {
            ID3D11ShaderReflectionVariable* var = buf->GetVariableByIndex(i);
            D3D11_SHADER_VARIABLE_DESC vd;//descriptor for the shader variable
            var->GetDesc(&vd);
            Uniform u;
            strcpy_s(u.Name, (const char*)vd.Name);;
            u.Offset = vd.StartOffset;
            u.Size   = vd.Size;
            UniformInfo[numUniformInfo++]=u;
        }
        UniformsSize = bufd.Size;
        UniformData  = (unsigned char*)OVR_ALLOC(bufd.Size);
    }

    void SetUniform(const char* name, int n, const float* v)
    {
        for (int i=0;i<numUniformInfo;i++)
        {
            if (!strcmp(UniformInfo[i].Name,name))
            {
                memcpy(UniformData + UniformInfo[i].Offset, v, n * sizeof(float));
                return;
            }
        }
    }
};
//------------------------------------------------------------
struct ImageBuffer
{
    ID3D11Texture2D *            Tex;
    ID3D11ShaderResourceView *   TexSv;
    ID3D11RenderTargetView *     TexRtv;
    ID3D11DepthStencilView *     TexDsv;
    Sizei                        Size;

    ImageBuffer::ImageBuffer(bool rendertarget, bool depth, Sizei size, int mipLevels = 1,
                             unsigned char * data = NULL) : Size(size)
    {
        D3D11_TEXTURE2D_DESC dsDesc;
        dsDesc.Width     = size.w;
        dsDesc.Height    = size.h;
        dsDesc.MipLevels = mipLevels;
        dsDesc.ArraySize = 1;
        dsDesc.Format    = depth ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;//if depth was specified make format 32 bit color or else 
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage     = D3D11_USAGE_DEFAULT;
        dsDesc.CPUAccessFlags = 0;
        dsDesc.MiscFlags      = 0;
        dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        if (rendertarget &&  depth) dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        if (rendertarget && !depth) dsDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        DX11.Device->CreateTexture2D(&dsDesc, NULL, &Tex);
        DX11.Device->CreateShaderResourceView(Tex, NULL, &TexSv);
        
        if (rendertarget &&  depth) DX11.Device->CreateDepthStencilView(Tex, NULL, &TexDsv);
        if (rendertarget && !depth) DX11.Device->CreateRenderTargetView(Tex, NULL, &TexRtv);
 
        if (data) // Note data is trashed, as is width and height //not sure what this comment means i assume it is not very efficient
        {
            for (int level=0; level < mipLevels; level++)
            {
                DX11.Context->UpdateSubresource(Tex, level, NULL, data, size.w * 4, size.h * 4);
                for(int j = 0; j < (size.h & ~1); j += 2)
                {
                    const uint8_t* psrc = data + (size.w * j * 4);
                    uint8_t*       pdest = data + ((size.w >> 1) * (j >> 1) * 4);
                    for(int i = 0; i < size.w >> 1; i++, psrc += 8, pdest += 4)
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
};
//-----------------------------------------------------
struct ShaderFill
{
    Shader             * VShader, *PShader;
    ImageBuffer        * OneTexture;
    ID3D11InputLayout  * InputLayout;
    ID3D11SamplerState * SamplerState;

    ShaderFill::ShaderFill(D3D11_INPUT_ELEMENT_DESC * VertexDesc, int numVertexDesc,
                           char* vertexShader, char* pixelShader, ImageBuffer * t, bool wrap=1)
        : OneTexture(t)
    {
        ID3D10Blob *blobData;
        D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &blobData, NULL);
        VShader = new Shader(blobData,0);
        DX11.Device->CreateInputLayout(VertexDesc, numVertexDesc,
                                       blobData->GetBufferPointer(), blobData->GetBufferSize(), &InputLayout);
        D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &blobData, NULL);
        PShader  = new Shader(blobData,1);

        D3D11_SAMPLER_DESC ss; memset(&ss, 0, sizeof(ss));
        ss.AddressU = ss.AddressV = ss.AddressW = wrap ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_BORDER;
        ss.Filter        = D3D11_FILTER_ANISOTROPIC;
        ss.MaxAnisotropy = 8;
        ss.MaxLOD        = 15;
        DX11.Device->CreateSamplerState(&ss, &SamplerState);
    }
};

//----------------------------------------------------------------
struct DataBuffer 
{
    ID3D11Buffer * D3DBuffer;
    size_t         Size;

    DataBuffer(D3D11_BIND_FLAG use, const void* buffer, size_t size) : Size(size)
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
    void Refresh(const void* buffer, size_t size)
    {
        D3D11_MAPPED_SUBRESOURCE map;
        DX11.Context->Map(D3DBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);   
        memcpy((void *)map.pData, buffer, size);
        DX11.Context->Unmap(D3DBuffer, 0);
    }
};

#endif
