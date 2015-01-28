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
	Shader(ID3D10Blob* s, int which_type);

	void SetUniform(const char* name, int n, const float* v);
};
//------------------------------------------------------------
struct ImageBuffer
{
    ID3D11Texture2D *            Tex;
    ID3D11ShaderResourceView *   TexSv;
    ID3D11RenderTargetView *     TexRtv;
    ID3D11DepthStencilView *     TexDsv;
    Sizei                        Size;

	ImageBuffer(bool rendertarget, bool depth, Sizei size, int mipLevels = 1,
		unsigned char * data = NULL);

};
//-----------------------------------------------------
struct ShaderFill
{
    Shader             * VShader, *PShader;
    ImageBuffer        * OneTexture;
    ID3D11InputLayout  * InputLayout;
    ID3D11SamplerState * SamplerState;

	ShaderFill::ShaderFill(D3D11_INPUT_ELEMENT_DESC * VertexDesc, int numVertexDesc,
		char* vertexShader, char* pixelShader, ImageBuffer * t, bool wrap = 1);
};

//----------------------------------------------------------------
struct DataBuffer 
{
    ID3D11Buffer * D3DBuffer;
    size_t         Size;

	DataBuffer(D3D11_BIND_FLAG use, const void* buffer, size_t size);

	void Refresh(const void* buffer, size_t size);

};

#endif
