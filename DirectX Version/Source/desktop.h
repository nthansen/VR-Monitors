#ifndef DESKTOP_H
#define DESKTOP_H

#include <windows.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include <dxgi1_2.h>
#include <sal.h>
#include <new>
#include <warning.h>
#include <DirectXMath.h>
#include "Win32_DX11AppUtil.h"

// FRAME_DATA holds information about an acquired frame
//
typedef struct FRAME_DATA
{
    ID3D11Texture2D* Frame;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;
    _Field_size_bytes_((MoveCount * sizeof(DXGI_OUTDUPL_MOVE_RECT)) + (DirtyCount * sizeof(RECT))) BYTE* MetaData;
    UINT DirtyCount;
    UINT MoveCount;
};

//
// Holds info about the pointer/cursor
//
typedef struct _PTR_INFO
{
	_Field_size_bytes_(BufferSize) BYTE* PtrShapeBuffer;
	DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo;
	POINT Position;
	bool Visible;
	UINT BufferSize;
	UINT WhoUpdatedPositionLast;
	LARGE_INTEGER LastTimeStamp;
} PTR_INFO;

typedef struct _THREAD_DATA
{
	UINT Output;
	INT OffsetX;
	INT OffsetY;
	PTR_INFO* PtrInfo;
} THREAD_DATA;



//
// FRA

class Desktop {
public:
    ID3D11Texture2D* desktopImage;
    ID3D11Texture2D* stage;
    ID3D11Texture2D* masterImage;
    ID3D11ShaderResourceView* masterView;
    ID3D11Device* Device;
    ID3D11DeviceContext* deviceContext;
    ShaderFill* masterFill;
    ImageBuffer * masterBuffer;

	PTR_INFO ptrInfo;

    Desktop();
    ~Desktop();

    void init(boolean newMonitor); // binds the desktop to the current thread
    //TODO: change return type to an enum
    _Success_(*Timeout == false && return == 0) int getFrame(_Out_ FRAME_DATA* Data, _Out_ bool* Timeout);
    int relaseFrame();
private:
    IDXGIOutputDuplication* desktop; //desktop object to get frame data from
    _Field_size_bytes_(MetaDataSize) BYTE* metaDataBuffer;
    UINT metaDataSize;
    UINT outputNumber;
    DXGI_OUTPUT_DESC OutputDesc;
    static int const timeout = 500; //desktop image grab timoute in ms
	HANDLE thread;
};


#endif