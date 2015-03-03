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
#include <stdio.h>

#define WINSTA_ALL (WINSTA_ACCESSCLIPBOARD | WINSTA_ACCESSGLOBALATOMS | WINSTA_CREATEDESKTOP | WINSTA_ENUMDESKTOPS |WINSTA_ENUMERATE | WINSTA_EXITWINDOWS |WINSTA_READATTRIBUTES | WINSTA_READSCREEN |WINSTA_WRITEATTRIBUTES | DELETE |READ_CONTROL | WRITE_DAC |WRITE_OWNER)

#define DESKTOP_ALL (DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW  | DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALPLAYBACK | DESKTOP_JOURNALRECORD | DESKTOP_READOBJECTS | DESKTOP_SWITCHDESKTOP | DESKTOP_WRITEOBJECTS | DELETE | READ_CONTROL | WRITE_DAC | WRITE_OWNER)

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)

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

class Desktop {
public:
    ID3D11Texture2D* desktopImage;
    ID3D11Texture2D* stage;
    ID3D11Texture2D* masterImage;
    ID3D11Texture2D* stageHandle;
    ID3D11ShaderResourceView* masterView;
    ID3D11Device* Device;
    ID3D11DeviceContext* deviceContext;
    ShaderFill* masterFill;
    ImageBuffer * masterBuffer;

	PTR_INFO ptrInfo;

	void newDesktop();
	HDESK mainDesktop;
    Desktop(int id);
    Desktop();
    ~Desktop();

    void init(int outputNumber); // binds the desktop to the current thread
    //TODO: change return type to an enum
    _Success_(*Timeout == false && return == 0) int getFrame(_Out_ FRAME_DATA* Data, _Out_ bool* Timeout);
    int relaseFrame();
private:
    int output;
    LPWSTR desktopName;
    bool initialized;
    IDXGIOutputDuplication* desktop; //desktop object to get frame data from
    _Field_size_bytes_(MetaDataSize) BYTE* metaDataBuffer;
    UINT metaDataSize;
    UINT outputNumber;
    DXGI_OUTPUT_DESC OutputDesc;
    static int const timeout = 500; //desktop image grab timoute in ms
	HANDLE thread;
};

#endif