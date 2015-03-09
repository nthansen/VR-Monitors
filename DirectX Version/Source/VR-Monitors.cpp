// This app renders a simple room, with right handed coord system :  Y->Up, Z->Back, X->Right
// 'W','A','S','D' and arrow keys to navigate.
// 1.  SDK-rendered is the simplest path (this file)
// 4.  Supporting D3D11 and utility code is in Win32_DX11AppUtil.h

// Choose whether the SDK performs rendering/distortion, or the application.
#define SDK_RENDER 1

#include "Win32_DX11AppUtil.h"			// Include Non-SDK supporting utilities
#include "scene.h"
#include "OVR_CAPI.h"					// Include the OculusVR SDK
#include "controlPanel.h"
#include "desktop.h"
#include "../3rdParty/ScreenGrab/ScreenGrab.h"


ovrHmd           HMD;					// The handle of the headset
ovrEyeRenderDesc EyeRenderDesc[2];		// Description of the VR.
ovrRecti         EyeRenderViewport[2];	// Useful to remember when varying resolution
ImageBuffer    * pEyeRenderTexture[2];	// Where the eye buffers will be rendered
ImageBuffer    * pEyeDepthBuffer[2];	// For the eye buffers to use when rendered
ovrPosef         EyeRenderPose[2];		// Useful to remember where the rendered eye originated
float            YawAtRender[2];		// Useful to remember where the rendered eye originated
float			 Yaw(3.141592f);		// Horizontal rotation of the player
Vector3f         Pos(0, 0, -1.0f);	// Position of player
int				 clock;

#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"                   // Include SDK-rendered code for the D3D version

//-------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
	HDESK currentDesktop = OpenInputDesktop(0, false, DESKTOP_READOBJECTS);
	wchar_t data[100]; //name should never be more than 100
	LPDWORD lpnLengthNeeded = new DWORD;
	bool result = GetUserObjectInformation(currentDesktop, UOI_NAME, data, (100 * sizeof(char)), lpnLengthNeeded);
	int length = (int)(*lpnLengthNeeded);

	wcscat(data, L" VR-Monitors");
    HANDLE hFirstExecution = CreateMutex(NULL, TRUE, L"VR-Monitors Original");
    DWORD createError = GetLastError();
    if (createError != ERROR_ALREADY_EXISTS) {
        WCHAR cmd[] = L"Desktops.exe";
        STARTUPINFOW si = { 0 };
        si.cb = sizeof(si);
        si.lpDesktop = L"Default";
       //a si.wShowWindow = SW_SHOW;
        PROCESS_INFORMATION pi;
        int result = CreateProcess(NULL, cmd, 0, 0, FALSE, NULL, NULL, NULL, &si, &pi);
        DWORD le = GetLastError();
    }

	HANDLE hSingleInstanceMutex = CreateMutex(NULL, TRUE, ((LPWSTR)(data)));
	DWORD dwError = GetLastError();
	if (dwError == ERROR_ALREADY_EXISTS)
	{
		// Application already lunched
		printf("running");
		return 0;
	}

    // Initializes LibOVR, and the Rift
    ovr_Initialize();
    HMD = ovrHmd_Create(0);
    clock = 0;
    const float PI = 3.1415972f;

    if (!HMD)
    {

        // If we didn't detect an Hmd, create a simulated one for debugging.
        HMD = ovrHmd_CreateDebug(ovrHmd_DK2);
        if (!HMD)
        {   // Failed Hmd creation.
            return 1;
        }
    }
    if (HMD->ProductName[0] == '\0')  MessageBoxA(NULL, "Rift detected, display not enabled.", "", MB_OK);

    // Setup Window and Graphics - use window frame if relying on Oculus driver
    bool windowed = (HMD->HmdCaps & ovrHmdCap_ExtendDesktop) ? false : true;
    if (!DX11.InitWindowAndDevice(hinst, Recti(HMD->WindowsPos, HMD->Resolution), windowed))
        return(0);

    DX11.SetMaxFrameLatency(1);//see declaration for better description, I wrote in the .h --brian
    ovrHmd_AttachToWindow(HMD, DX11.Window, NULL, NULL);
    ovrHmd_SetEnabledCaps(HMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

    // Start the sensor which informs of the Rift's pose and motion
    ovrHmd_ConfigureTracking(HMD, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection |
        ovrTrackingCap_Position, 0);

    // Make the eye render buffers (caution if actual size < requested due to HW limits). 
    for (int eye = 0; eye < 2; eye++)
    {
        Sizei idealSize = ovrHmd_GetFovTextureSize(HMD, (ovrEyeType)eye,
            HMD->DefaultEyeFov[eye], 1.0f);
        pEyeRenderTexture[eye] = new ImageBuffer(true, false, idealSize);
        pEyeDepthBuffer[eye] = new ImageBuffer(true, true, pEyeRenderTexture[eye]->Size);
        EyeRenderViewport[eye].Pos = Vector2i(0, 0);
        EyeRenderViewport[eye].Size = pEyeRenderTexture[eye]->Size;
    }

    ovrD3D11Config d3d11cfg;
    d3d11cfg.D3D11.Header.API = ovrRenderAPI_D3D11;
    d3d11cfg.D3D11.Header.BackBufferSize = Sizei(HMD->Resolution.w, HMD->Resolution.h);
    d3d11cfg.D3D11.Header.Multisample = 1;
    d3d11cfg.D3D11.pDevice = DX11.Device;
    d3d11cfg.D3D11.pDeviceContext = DX11.Context;
    d3d11cfg.D3D11.pBackBufferRT = DX11.BackBufferRT;
    d3d11cfg.D3D11.pSwapChain = DX11.SwapChain;

    if (!ovrHmd_ConfigureRendering(HMD, &d3d11cfg.Config,
        ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette |
        ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
        HMD->DefaultEyeFov, EyeRenderDesc))
    {
        return(1);
    }

    // Create the room model
    Scene roomScene = Scene(); // Can simplify scene further with parameter if required.
	//Vector3f mpos = roomScene.Models[0]->Pos; roomScene.Models[0]->Pos = roomScene.Models[0]->Mat.Transform(Vector3f(-2, 0, 0));
	//need to pass in view and proj to control panel to render model while transitioning
	Matrix4f view;
	Matrix4f proj;
    controlPanel.createControlPanel(hinst, &roomScene, &Pos, &HMD, &Yaw, &view, &proj);
	int				 count = 1;
    // MAIN LOOP
    // =========

	while (!(DX11.Key['Q'] && DX11.Key[VK_CONTROL]) && !DX11.Key[VK_ESCAPE] && !controlPanel.getCloseApp())
	{
		if (roomScene.doRender) {
			DX11.HandleMessages();

			controlPanel.updateControlPanel();

			// remove the health/warning display
			ovrHmd_DismissHSWDisplay(HMD);

			float       speed = 1.0f; // Can adjust the movement speed. 
			int         timesToRenderScene = 1;    // Can adjust the render burden on the app.
			ovrVector3f useHmdToEyeViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset,
				EyeRenderDesc[1].HmdToEyeViewOffset };
			// Start timing
			ovrHmd_BeginFrame(HMD, 0);

			// Update the clock, used by some of the features
			clock++;

			// Keyboard inputs to adjust player orientation
			if (DX11.Key[VK_LEFT])  Yaw += 0.02f;
			if (DX11.Key[VK_RIGHT]) Yaw -= 0.02f;

			// Keyboard inputs to adjust player position
			if (DX11.Key['W'] || DX11.Key[VK_UP])	Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(0, 0, -speed*0.05f));
			if (DX11.Key['S'] || DX11.Key[VK_DOWN])	Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(0, 0, +speed*0.05f));
			if (DX11.Key['D'])						Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(+speed*0.05f, 0, 0));
			if (DX11.Key['A'])						Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(-speed*0.05f, 0, 0));
			//spawn a new monitor
			if (DX11.Key['M'] && clock % 6 == 0){//restricts multiple monitors being made
				//could probably fix this by adding a splash screen to confirm add monitor
				roomScene.addMonitor(Yaw, Pos);
			}
			// just so it'd give some time before switching between each texture
			// replaces the shader fill to the new shaderfill
			for (int i = 0; i < roomScene.num_monitors; i++){
				bool timedout;
				FRAME_DATA frame;
				roomScene.Monitors[i]->desktop->relaseFrame();

				roomScene.Monitors[i]->desktop->getFrame(&frame, &timedout);
				if (frame.Frame != nullptr && !timedout) {

					// the frame that comes from the desktop duplication api is not sharable so we must copy it to a controled resource
					D3D11_TEXTURE2D_DESC frameDesc;
					frame.Frame->GetDesc(&frameDesc);
					HRESULT hr;
					roomScene.Monitors[i]->desktop->deviceContext->CopyResource(roomScene.Monitors[i]->desktop->stage, frame.Frame);
                    if (frame.FrameInfo.LastMouseUpdateTime.QuadPart != 0 && frame.FrameInfo.PointerPosition.Visible) {
                        //mouse has been updated, draw onto stage
                        roomScene.Monitors[i]->desktop->deviceContext->CopySubresourceRegion(
                            roomScene.Monitors[i]->desktop->stage,
                            0,
                            roomScene.Monitors[i]->desktop->pointer.Position.x,
                            roomScene.Monitors[i]->desktop->pointer.Position.y,
                            0,
                            roomScene.Monitors[i]->desktop->pointerImage,
                            0,
                            NULL);
                    }
					// we capture a shared handle from the staging resource 
					HANDLE Hnd(NULL);
					IDXGIResource* DXGIResource = nullptr;
					hr = roomScene.Monitors[i]->desktop->stage->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&DXGIResource));
					DXGIResource->GetSharedHandle(&Hnd);
					DXGIResource->Release();
					DXGIResource = nullptr;
					ID3D11Texture2D* tmp;
					hr = DX11.Device->OpenSharedResource(Hnd, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&tmp));

					// using the shared handle we copy the data to the image bound to the render view
					DX11.Context->CopyResource(roomScene.Monitors[i]->desktop->masterImage, tmp);
				}
				roomScene.Monitors[i]->desktop->relaseFrame();
			}

			// accesses the actual texture in the shaderfill and switches them out
			//roomScene.Models[0]->Fill->OneTexture = roomScene.generated_texture[clock % 5]->OneTexture;

			// figuring out how model rotation works
			if (DX11.Key['T']) {
				//rotate the object about the y-axis (or very close) based on the depth of the object at the angle described
				//since the object spawns in front of us on the z axis and we are now facing the direction of positive x axis
				//we must offset this to rotate negative pi radians so the object will be in front of us
				roomScene.Models[0]->Pos = Pos;
				roomScene.Models[0]->Rot = Quatf(Vector3f(0 , 1 , 0), -PI + Yaw);

			}

			if (DX11.Key['Z']){//&&clock%12==0) {
				//rotate the object about the y-axis (or very close) based on the depth of the object at the angle described
				//since the object spawns in front of us on the z axis and we are now facing the direction of positive x axis
				//we must offset this to rotate negative pi radians so the object will be in front of us
				//roomScene.Models[0]->Pos = Pos;
				Model *mod = roomScene.Models[0];
				//mod->Pos.x = 1;
				//mod->Mat = Matrix4f::Matrix4() * mod->Mat.Translation(Vector3f(4, 0, 0));
				//roomScene.Models[0]->Rot = roomScene.Models[0]->Rot.Nlerp(Quatf(Vector3f(0, 1, 0), PI / 2 * count++), .9);
				roomScene.Models[0]->Rot = Quatf(Vector3f(0, 1, 0), PI / 4 * count++);
				//Matrix4f  modmat = mod->GetMatrix();
				//roomScene.Models[0]->Rot = Quatf(Vector3f(0, 1, 0), PI / 2);
				//mod->Pos = modmat.Transform(Vector3f(-2, 0, 0));
				//mod->Pos = Vector3f(-2, 0, 0);

			}
			if (DX11.Key['X']) {
				while (roomScene.Models[0]->Rot.Angle(Quatf(Vector3f(0, .00001, 0), 3.14159 / 2 * 1)) > 0.01){
					roomScene.Models[0]->Rot = roomScene.Models[0]->Rot.Nlerp(Quatf(Vector3f(0, .000001, 0), PI / 2 * 1), .9);
					roomScene.Render(view, proj);
				}
				Model *mod = roomScene.Models[0];
				Matrix4f  modmat = mod->GetMatrix();
				mod->Pos = modmat.Transform(Vector3f(-2, 0, 0));
			}
			// shows how to select a model and mess with it
			/*
			// Animate the cube
			if (speed)
			roomScene.Models[0]->Pos = Vector3f(9 * sin(0.01f*clock), 3, 9 * cos(0.01f*clock));
			*/

			// Get both eye poses simultaneously, with IPD offset already included. 
			ovrPosef temp_EyeRenderPose[2];
			ovrHmd_GetEyePoses(HMD, 0, useHmdToEyeViewOffset, temp_EyeRenderPose, NULL);

			// Render the two undistorted eye views into their render buffers.  
			for (int eye = 0; eye < 2; eye++)
			{
				ImageBuffer * useBuffer = pEyeRenderTexture[eye];
				ovrPosef    * useEyePose = &EyeRenderPose[eye];
				float       * useYaw = &YawAtRender[eye];
				bool          clearEyeImage = true;
				bool          updateEyeImage = true;

				if (clearEyeImage)
					DX11.ClearAndSetRenderTarget(useBuffer->TexRtv,
					pEyeDepthBuffer[eye], Recti(EyeRenderViewport[eye]));
				if (updateEyeImage)
				{
					// Write in values actually used (becomes significant in Example features)
					*useEyePose = temp_EyeRenderPose[eye];
					*useYaw = Yaw;

					// Get view and projection matrices (note near Z to reduce eye strain)
					Matrix4f rollPitchYaw = Matrix4f::RotationY(Yaw);
					Matrix4f finalRollPitchYaw = rollPitchYaw *Matrix4f(useEyePose->Orientation);
					Vector3f finalUp = finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
					Vector3f finalForward = finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
					Vector3f shiftedEyePos = Pos + rollPitchYaw.Transform(useEyePose->Position);

					Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
					Matrix4f proj = ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, 0.2f, 1000.0f, true);

					// Render the scene
					for (int t = 0; t < timesToRenderScene; t++)
						roomScene.Render(view, proj.Transposed());
				}
			}

			// Do distortion rendering, Present and flush/sync
			ovrD3D11Texture eyeTexture[2]; // Gather data for eye textures 
			for (int eye = 0; eye < 2; eye++)
			{
				eyeTexture[eye].D3D11.Header.API = ovrRenderAPI_D3D11;
				eyeTexture[eye].D3D11.Header.TextureSize = pEyeRenderTexture[eye]->Size;
				eyeTexture[eye].D3D11.Header.RenderViewport = EyeRenderViewport[eye];
				eyeTexture[eye].D3D11.pTexture = pEyeRenderTexture[eye]->Tex;
				eyeTexture[eye].D3D11.pSRView = pEyeRenderTexture[eye]->TexSv;
			}
			ovrHmd_EndFrame(HMD, EyeRenderPose, &eyeTexture[0].Texture);
		}

	}
    // Release and close down
    ovrHmd_Destroy(HMD);
    ovr_Shutdown();
    DX11.ReleaseWindow(hinst);
    controlPanel.~ControlPanel();

    return(0);
}



