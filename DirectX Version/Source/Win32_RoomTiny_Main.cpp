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

ovrHmd           HMD;					// The handle of the headset
ovrEyeRenderDesc EyeRenderDesc[2];		// Description of the VR.
ovrRecti         EyeRenderViewport[2];	// Useful to remember when varying resolution
ImageBuffer    * pEyeRenderTexture[2];	// Where the eye buffers will be rendered
ImageBuffer    * pEyeDepthBuffer[2];	// For the eye buffers to use when rendered
ovrPosef         EyeRenderPose[2];		// Useful to remember where the rendered eye originated
float            YawAtRender[2];		// Useful to remember where the rendered eye originated
float            Yaw(0);		// Horizontal rotation of the player
Vector3f         Pos(0.0f,0.0f,0.0f);	// Position of player
int				 clock;

#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"                   // Include SDK-rendered code for the D3D version


//-------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{

    // Initializes LibOVR, and the Rift
    ovr_Initialize();
    HMD = ovrHmd_Create(0);
	clock = 0;
	static const float PI = 3.1415972f;

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
    for (int eye=0; eye<2; eye++)
    {
        Sizei idealSize             = ovrHmd_GetFovTextureSize(HMD, (ovrEyeType)eye,
                                                               HMD->DefaultEyeFov[eye], 1.0f);
        pEyeRenderTexture[eye]      = new ImageBuffer(true, false, idealSize);
        pEyeDepthBuffer[eye]        = new ImageBuffer(true, true, pEyeRenderTexture[eye]->Size);
        EyeRenderViewport[eye].Pos  = Vector2i(0, 0);
        EyeRenderViewport[eye].Size = pEyeRenderTexture[eye]->Size;
    }

    ovrD3D11Config d3d11cfg;
    d3d11cfg.D3D11.Header.API            = ovrRenderAPI_D3D11;
    d3d11cfg.D3D11.Header.BackBufferSize = Sizei(HMD->Resolution.w, HMD->Resolution.h);
    d3d11cfg.D3D11.Header.Multisample    = 1;
    d3d11cfg.D3D11.pDevice               = DX11.Device;
    d3d11cfg.D3D11.pDeviceContext        = DX11.Context;
    d3d11cfg.D3D11.pBackBufferRT         = DX11.BackBufferRT;
    d3d11cfg.D3D11.pSwapChain            = DX11.SwapChain;

	if (!ovrHmd_ConfigureRendering(HMD, &d3d11cfg.Config,
		ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette |
		ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive,
		HMD->DefaultEyeFov, EyeRenderDesc))
	{
		return(1);
	}

    // Create the room model
    Scene roomScene = Scene(); // Can simplify scene further with parameter if required.

	controlPanel.createControlPanel(hinst, &roomScene, &Pos, &HMD);

    // MAIN LOOP
    // =========
	while (!controlPanel.getCloseApp())
	{
		DX11.HandleMessages();

		// remove the health/warning display
		ovrHmd_DismissHSWDisplay(HMD);

		float       speed = 3.0f; // Can adjust the movement speed. 
		int         timesToRenderScene = 1;    // Can adjust the render burden on the app.
		ovrVector3f useHmdToEyeViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset,
			EyeRenderDesc[1].HmdToEyeViewOffset };
		// Start timing
		ovrHmd_BeginFrame(HMD, 0);

		// Update the clock, used by some of the features
		clock++;

		// Keyboard inputs to adjust player orientation
		if (DX11.Key[VK_LEFT])  Yaw += 0.08f;
		if (DX11.Key[VK_RIGHT]) Yaw -= 0.08f;

		// Keyboard inputs to adjust player position
		if (DX11.Key['W'] || DX11.Key[VK_UP])	Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(0, 0, -speed*0.05f));
		if (DX11.Key['S'] || DX11.Key[VK_DOWN])	Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(0, 0, +speed*0.05f));
		if (DX11.Key['D'])						Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(+speed*0.05f, 0, 0));
		if (DX11.Key['A'])						Pos += Matrix4f::RotationY(Yaw).Transform(Vector3f(-speed*0.05f, 0, 0));
		//spawn a new monitor
		if (DX11.Key['M'] && clock % 6 == 0){//restricts multiple monitors being made
			//could probably fix this by adding a splash screen to confirm add monitor
			roomScene.addMonitor();
		}
		// just so it'd give some time before switching between each texture
		if (clock % 24 == 0) {
			// replaces the shader fill to the new shaderfill
			roomScene.Models[0]->Fill = roomScene.generated_texture[clock % 5];
			// accesses the actual texture in the shaderfill and switches them out
			//roomScene.Models[0]->Fill->OneTexture = roomScene.generated_texture[clock % 5]->OneTexture;
		}
		// figuring out how model rotation works
		if (DX11.Key['T']) {
			//rotate the object about the y-axis (or very close) based on the depth of the object at the angle described
			//since the object spawns in front of us on the z axis and we are now facing the direction of positive x axis
			//we must offset this to rotate negative pi radians so the object will be in front of us
			roomScene.Models[0]->Rot = Quatf(Vector3f(0, Pos.z == 0 ? .001 : Pos.z, 0), -PI + Yaw);
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
				Matrix4f rollPitchYaw =			Matrix4f::RotationY(Yaw);
				Matrix4f finalRollPitchYaw =	rollPitchYaw *Matrix4f(useEyePose->Orientation);
				Vector3f finalUp =				finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
				Vector3f finalForward =			finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
				Vector3f shiftedEyePos =		Pos + rollPitchYaw.Transform(useEyePose->Position);

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

    // Release and close down
    ovrHmd_Destroy(HMD);
    ovr_Shutdown();
	DX11.ReleaseWindow(hinst);
	controlPanel.~ControlPanel();

    return(0);
}

