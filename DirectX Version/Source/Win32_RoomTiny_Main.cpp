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

#include "../3rdParty/SimpleMath.h"		//wrapper for directxmath used for collisions
#include <DirectXMath.h>
//#include <DirectXColors.h>
#include <DirectXCollision.h>
using namespace DirectX::SimpleMath;
using namespace DirectX;
ovrHmd           HMD;					// The handle of the headset
ovrEyeRenderDesc EyeRenderDesc[2];		// Description of the VR.
ovrRecti         EyeRenderViewport[2];	// Useful to remember when varying resolution
ImageBuffer    * pEyeRenderTexture[2];	// Where the eye buffers will be rendered
ImageBuffer    * pEyeDepthBuffer[2];	// For the eye buffers to use when rendered
ovrPosef         EyeRenderPose[2];		// Useful to remember where the rendered eye originated
float            YawAtRender[2];		// Useful to remember where the rendered eye originated
float			Yaw(3.141592f);			// Horizontal rotation of the player
Vector3f         Pos(0.0f,0.0f,0.0f);	// Position of player
int				 clock;
const float CAMERA_SPACING = 50.f;		//camera distance

#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"                   // Include SDK-rendered code for the D3D version

//struct CollisionRay
//{
//	XMVECTOR origin;
//	XMVECTOR direction;
//};
//
//struct CollisionBox
//{
//	BoundingOrientedBox obox;
//	ContainmentType collision;
//};
//
//struct CollisionAABox
//{
//	BoundingBox aabox;
//	ContainmentType collision;
//};
//
////primary collision objects
//BoundingBox g_PrimaryAABox;
//CollisionRay g_PrimaryRay;
//
////secondary collision objects
//CollisionBox        g_SecondaryOrientedBox;
//CollisionAABox      g_SecondaryAABox;
//
//// Ray testing results display object
//CollisionAABox g_RayHitResultBox;
// --------------------------------------------------------------------------------------
// Test collisions between pairs of collision objects using XNACollision functions
//--------------------------------------------------------------------------------------
//void Collide()
//{
//	// test collisions between objects and ray
//	float fDistance = -1.0f;
//
//	float fDist;
//	if (g_SecondaryOrientedBox.obox.Intersects(g_PrimaryRay.origin, g_PrimaryRay.direction, fDist))
//	{
//		fDistance = fDist;
//		g_SecondaryOrientedBox.collision = INTERSECTS;
//	}
//	else
//		g_SecondaryOrientedBox.collision = DISJOINT;
//
//	if (g_SecondaryAABox.aabox.Intersects(g_PrimaryRay.origin, g_PrimaryRay.direction, fDist))
//	{
//		fDistance = fDist;
//		g_SecondaryAABox.collision = INTERSECTS;
//	}
//	else
//		g_SecondaryAABox.collision = DISJOINT;
//	// If one of the ray intersection tests was successful, fDistance will be positive.
//	// If so, compute the intersection location and store it in g_RayHitResultBox.
//	if (fDistance > 0)
//	{
//		// The primary ray's direction is assumed to be normalized.
//		XMVECTOR HitLocation = XMVectorMultiplyAdd(g_PrimaryRay.direction, XMVectorReplicate(fDistance),
//			g_PrimaryRay.origin);
//		XMStoreFloat3(&g_RayHitResultBox.aabox.Center, HitLocation);
//		g_RayHitResultBox.collision = INTERSECTS;
//	}
//	else
//	{
//		g_RayHitResultBox.collision = DISJOINT;
//	}
//}

//--------------------------------------------------------------------------------------
// Returns the color based on the collision result and the gruop number.
// Frustum tests (group 0) return 0, 1, or 2 for outside, partially inside, and fully inside;
// all other tests return 0 or 1 for no collision or collision.
//--------------------------------------------------------------------------------------
//inline XMVECTOR GetCollisionColor(ContainmentType collision, int groupnumber)
//{
//	// special case: a value of 1 for groups 1 and higher needs to register as a full collision
//	if (groupnumber >= 3 && collision > 0)
//		collision = CONTAINS;
//
//	switch (collision)
//	{
//	case DISJOINT:      return Colors::Green;
//	case INTERSECTS:    return Colors::Yellow;
//	case CONTAINS:
//	default:            return Colors::Red;
//	}
//}
//-------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
	// DirectXMath uses SSE/SSE2 instructions on Windows. We should verify the CPU supports these instructions
	// as early in the program as possible
	if (!XMVerifyCPUSupport())
	{
		MessageBox(NULL, TEXT("This application requires the processor support SSE2 instructions."),
			TEXT("Collision"), MB_OK | MB_ICONEXCLAMATION);
		return -1;
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
	Matrix4f view;
	Matrix4f proj;
	Vector3f finalForward;
	Vector3f shiftedEyePos;
	Vector3f at = shiftedEyePos + finalForward;
	controlPanel.createControlPanel(hinst, &roomScene, &Pos, &HMD, &Yaw, &view, &proj);

    // MAIN LOOP
    // =========
	while (!(DX11.Key['Q'] && DX11.Key[VK_CONTROL]) && !DX11.Key[VK_ESCAPE] && !controlPanel.getCloseApp())
	{
		DX11.HandleMessages();

		controlPanel.updateControlPanel();

		// remove the health/warning display
		ovrHmd_DismissHSWDisplay(HMD);

		float       speed = 4.0f; // Can adjust the movement speed. 
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
			roomScene.addMonitor(Yaw, Pos);
		}
		// just so it'd give some time before switching between each texture
		if (clock % 24 == 0) {
			// replaces the shader fill to the new shaderfill
			roomScene.Models[0]->Fill = roomScene.generated_texture[clock % 5];
			// accesses the actual texture in the shaderfill and switches them out
			//roomScene.Models[0]->Fill->OneTexture = roomScene.generated_texture[clock % 5]->OneTexture;
		}	// figuring out how model rotation works
		if (DX11.Key['T']) {
			//rotate the object about the y-axis (or very close) based on the depth of the object at the angle described
			//since the object spawns in front of us on the z axis and we are now facing the direction of positive x axis
			//we must offset this to rotate negative pi radians so the object will be in front of us
			//roomScene.Models[0]->Pos = Pos;
			//roomScene.Models[0]->Rot = Quatf(Vector3f(0, Pos.y == 0 ? .001 : Pos.y, 0), -PI + Yaw);
			Model *asdf = roomScene.Monitors[0];
			//Vector3f xnorm = Vector3f(view.M[0][3], view.M[1][3], view.M[2][3]);
			//Vector3f ynorm = Vector3f(view.M[2][1], view.M[2][2], view.M[2][3]);
			//Vector3f znorm = xnorm.Cross(ynorm);
			//Matrix4f onorm = 			
			//Vector3f normal = xnorm.Normalized();
			//at;
			//shiftedEyePos;
			//int jk = 1;
			//Matrix4f scale = Matrix4f::Scaling(2);
			//asdf->GetMatrix();
			//asdf->Pos = (asdf->Mat).Transform(Vector3f(0, .1, 0));

			////using directxmath.h and collision.h
			//bool hit;
			//// Set up the primary ray
			//g_PrimaryRay.origin = XMVectorSet(Pos.x,Pos.y,Pos.z,1);
			//g_PrimaryRay.direction = g_XMIdentityR2;
			//// Set up ray hit result box
			//g_RayHitResultBox.aabox.Center = XMFLOAT3(0, 0, 0);
			//g_RayHitResultBox.aabox.Extents = XMFLOAT3(0.05f, 0.05f, 0.05f);

			////initialize objects
			//const XMVECTOR XMZero = XMVectorZero();
			//g_SecondaryOrientedBox.obox.Center = XMFLOAT3(0, 0, 0);
			//g_SecondaryOrientedBox.obox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
			//g_SecondaryOrientedBox.obox.Orientation = XMFLOAT4(0, 0, 0, 1);
			//g_SecondaryOrientedBox.collision = DISJOINT;

			//g_SecondaryAABox.aabox.Center = XMFLOAT3(0, 0, 0);
			//g_SecondaryAABox.aabox.Extents = XMFLOAT3(0.5f, 0.5f, 0.5f);
			//g_SecondaryAABox.collision = DISJOINT;

			//// Set up ray hit result box
			//g_RayHitResultBox.aabox.Center = XMFLOAT3(0, 0, 0);
			//g_RayHitResultBox.aabox.Extents = XMFLOAT3(0.05f, 0.05f, 0.05f);
			//
			//// animate primary ray (this is the only animated primary object)
			//g_PrimaryRay.direction = XMVectorSet(sinf(clock * 3), 0, cosf(clock * 3), 0);

			//// Draw results of ray-object intersection, if there was a hit this frame
			//if (g_RayHitResultBox.collision != DISJOINT)
			//	hit = true;


			//trying to rotate a direction vector using quaternion but first lets get the ray right
			//XMVector3Rotate()
			//using simplemath
			// Here's a RH example
			//SimpleMath::Vector3 dPos = SimpleMath::Vector3(Pos.x, Pos.y, Pos.z);//position for simplemath
			////Vector3f x = asdf->Rot*Pos.x;
			////XMVECTOR bPos = XMLoadvect XMVECTOR(2, 3, 4);
			//SimpleMath::Vector3 dir = SimpleMath::Vector3(sinf(Yaw), 0, cosf(Yaw-PI));
			//asdf->Rot;
			//bool hit;
			////SimpleMath::Vector3 eye, target, up;
			//
			//	//Matrix mView = Matrix::CreateLookAt(eye, target, up);
			//Ray cast = Ray(dPos, dir);
			//	cast.position = dPos; //see if we can hit the plane
			//	//SimpleMath::Plane _plane = SimpleMath::Plane(0,0,2,20);
			//	DirectX::BoundingOrientedBox mbox = BoundingOrientedBox(XMFLOAT3(0, 0, 1), XMFLOAT3(.4f, .4f, 0.0f), XMFLOAT4(0,0,1,0));
			//	//SimpleMath::Vector3 normal = _plane.Normal();
			//	float _dist = 0.0f;
			//	//bool hit = cast.Intersects(mbox,_dist);
			//	if (mbox.Intersects(cast.position, cast.direction, _dist)){
			//		hit = true;
			//	}
			/*if (hit){
				int i = 0;
				}*/
			bool hit = true;//stores whether we hit something
			Model *target = nullptr;//model we will find
			//save the direction to cast in dir and position to start from in dpos
			SimpleMath::Vector3 dPos = SimpleMath::Vector3(Pos.x, Pos.y, Pos.z);//position for simplemath
			SimpleMath::Vector3 dir = SimpleMath::Vector3(sinf(Yaw), 0, cosf(Yaw - PI));
			//ray is really only a container holding the origin and direction to cast
			SimpleMath::Ray cast = SimpleMath::Ray(dPos, dir);
			//crate a bounding box around each monitor to see if we hit one
			//as of now the second parameter is simply the distance from the center of the box to each edge x,y,z,w
			//magic number .4f is the distance from the center in x,y,z boxes center will be different

			//this is the logic that works everything else is crap
			DirectX::BoundingOrientedBox mbox = BoundingOrientedBox(XMFLOAT3(0, 0, 1), XMFLOAT3(.4f, .4f, 0.0f), XMFLOAT4(sinf(Yaw), 0, cosf(Yaw - PI), 0));
			float _dist = 0.0f;
			for (int i = 0; i < roomScene.num_monitors; i++){
				Model *temp = roomScene.Monitors[i];
				DirectX::BoundingOrientedBox mbox = BoundingOrientedBox(XMFLOAT3(temp->Pos.x, temp->Pos.y, temp->Pos.z), XMFLOAT3(.4f, .4f, 0.0f), XMFLOAT4(sinf(Yaw), 0, cosf(Yaw - PI), 0));

				if (mbox.Intersects(cast.position, cast.direction, _dist)){
					//					*target = *temp; //causes error but may want to save reference
					while (controlPanel.movingMonitor){
						temp->Pos = Pos;
						temp->Rot = Quatf(Vector3f(0, .001, 0), -PI + Yaw);
					}
				}
			}

			//// Here' is a LH example of same thing which relies on
			//// Vector3 and Matrix conversion operators
			//	SimpleMath::Vector3 eye, target, up;
			//	Matrix mView = XMMatrixLookAtLH(eye, target, up);



		}
		if (DX11.Key['R']) {
			roomScene.Models[0]->Pos = roomScene.Models[0]->Pos.Lerp(Pos, .6);
			roomScene.Models[0]->Rot = roomScene.Models[0]->Rot.Nlerp(Quatf(Vector3f(0, Pos.y == 0 ? .001 : Pos.y, 0), -PI + Yaw), .7);
		}
		//moves 90 degrees CCW
		if (DX11.Key['C'] && clock % 12 == 0) {
			Yaw += PI / 2;
		}
		//sets the picked monitor in controlPanel
		//must use to set the monitor then move with the button

			if (DX11.Key['P'] && clock % 12 == 0) {
				int monitorNum = roomScene.pickMonitor(Pos, Yaw);
				controlPanel.pickedMonitor = monitorNum;
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

				view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
				proj = ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, 0.2f, 1000.0f, true);
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

