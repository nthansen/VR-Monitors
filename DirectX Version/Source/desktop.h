#ifndef DESKTOP_H
#define DESKTOP_H

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <sal.h>
#include <new>
#include <warning.h>
#include <DirectXMath.h>
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

class Desktop {
public :
    ID3D11Texture2D* desktopImage;
    ID3D11Texture2D* masterImage;

    ID3D11Device* Device;
    ID3D11DeviceContext* deviceContext;


    Desktop();
    ~Desktop();

    void init(); // binds the desktop to the current thread
    //TODO: change return type to an enum
    _Success_(*Timeout == false && return == 0) int getFrame(_Out_ FRAME_DATA* Data, _Out_ bool* Timeout);
    int relaseFrame();
private:
    IDXGIOutputDuplication* desktop; //desktop object to get frame data from
    _Field_size_bytes_(MetaDataSize) BYTE* MetaDataBuffer;
    UINT MetaDataSize;
    UINT OutputNumber;
    DXGI_OUTPUT_DESC OutputDesc;
    static int const timeout = 500; //desktop image grab timoute in ms
};


#endif