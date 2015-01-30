/************************************************************************************

Filename    :   OculusWorldDemo.cpp
Content     :   First-person view test application for Oculus Rift - Implementation
Created     :   October 4, 2012
Authors     :   Michael Antonov, Andrew Reisse, Steve LaValle, Dov Katz
				Peter Hoff, Dan Goodman, Bryan Croteau                

Copyright   :   Copyright 2012 Oculus VR, LLC. All Rights reserved.

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

#include "OculusWorldDemo.h"
#include "Kernel/OVR_Threads.h"
#include "Util/Util_SystemGUI.h"


OVR_DISABLE_MSVC_WARNING(4996) // "scanf may be unsafe"

//-------------------------------------------------------------------------------------
// ***** OculusWorldDemoApp

OculusWorldDemoApp::OculusWorldDemoApp() :
    pRender(0),
    RenderParams(),
    WindowSize(1280,800),
    ScreenNumber(0),
    FirstScreenInCycle(0),
    SupportsSrgb(true),             // May be proven false below.
    SupportsMultisampling(true),    // May be proven false below.

    LastVisionProcessingTime(0.),
    VisionTimesCount(0),
    VisionProcessingSum(0.),
    VisionProcessingAverage(0.),

    DrawEyeTargets(NULL),
    Hmd(0),
    StartTrackingCaps(0),
    UsingDebugHmd(false),

    FrameCounter(0),
	TotalFrameCounter(0),
    SecondsPerFrame(0.f),
    FPS(0.f),
    LastFpsUpdate(0.0),
    LastUpdate(0.0),

    MainFilePath(),
    CollisionModels(),
    GroundCollisionModels(),

    LoadingState(LoadingState_Frame0),
    HaveVisionTracking(false),
    HavePositionTracker(false),
    HaveHMDConnected(false),

    ThePlayer(),
    View(),
    MainScene(),
    LoadingScene(),

    HmdFrameTiming(),
    HmdStatus(0),

    NotificationTimeout(0.0),

    HmdSettingsChanged(false),

	EnableSensor(true),
    PositionTrackingScale(1.0f),
    DesiredPixelDensity(1.0f),
    FovSideTanMax(1.0f), // Updated based on Hmd.
    FovSideTanLimit(1.0f), // Updated based on Hmd.
    FadedBorder(true),

    TimewarpEnabled(true),
    TimewarpRenderIntervalInSeconds(0.0f),
    ComputeShaderEnabled(false),

    CenterPupilDepthMeters(0.05f),
    VsyncEnabled(true),
    MultisampleEnabled(true),

    IsLowPersistence(true),
    DynamicPrediction(true),
    PositionTrackingEnabled(true),
	PixelLuminanceOverdrive(true),
    HqAaDistortion(true),
    MirrorToWindow(true),

    DistortionClearBlue(0),
    ShiftDown(false),
    CtrlDown(false),

    SceneMode(Scene_World),
    TextScreen(Text_None),
    Menu(),
    Profiler(),
    ExceptionHandler()
{
    EyeRenderSize[0] = EyeRenderSize[1] = Sizei(0);

    // EyeRenderDesc[], EyeTexture[] : Initialized in CalculateHmdValues()
}

OculusWorldDemoApp::~OculusWorldDemoApp()
{
    CleanupDrawTextFont();

    if (Hmd)
    {
        ovrHmd_Destroy(Hmd);
        Hmd = 0;
    }
	    
	CollisionModels.ClearAndRelease();
	GroundCollisionModels.ClearAndRelease();

    ovr_Shutdown();
}



int OculusWorldDemoApp::OnStartup(int argc, const char** argv)
{
    OVR::Thread::SetCurrentThreadName("OWDMain");

    // *** Setup exception handler

    ExceptionHandler.SetExceptionListener(this, 0);
    ExceptionHandler.SetExceptionPaths("default", "default");
    ExceptionHandler.EnableReportPrivacy(false); // If we were collecting these reports then we need to get user permission in order to enable disable privacy.
    ExceptionHandler.Enable(true);
    

    // *** Oculus HMD & Sensor Initialization

    // Create DeviceManager and first available HMDDevice from it.
    // Sensor object is created from the HMD, to ensure that it is on the
    // correct device.

    #if defined(OVR_OS_WIN32)
        OVR::Thread::SetCurrentPriority(Thread::HighestPriority);
    
        if(OVR::Thread::GetCPUCount() >= 4) // Don't do this unless there are at least 4 processors, otherwise the process could hog the machine.
        {
            SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        }
    #endif

    ovr_Initialize();

	Hmd = ovrHmd_Create(0);

	if (!Hmd)
	{
        Menu.SetPopupMessage("Unable to create HMD: %s", ovrHmd_GetLastError(NULL));

		// If we didn't detect an Hmd, create a simulated one for debugging.
		Hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		UsingDebugHmd = true;
		if (!Hmd)
		{   // Failed Hmd creation.
			return 1;
		}
	}
    
    if (Hmd->HmdCaps & ovrHmdCap_ExtendDesktop)
    {
        WindowSize = Hmd->Resolution;
    }
    else
    {
        // In Direct App-rendered mode, we can use smaller window size,
        // as it can have its own contents and isn't tied to the buffer.
        WindowSize = Sizei(1100, 618);//Sizei(960, 540); avoid rotated output bug.
    }


    // ***** Setup System Window & rendering.

    if (!SetupWindowAndRendering(argc, argv))
    {
        return 1;
    }

    NotificationTimeout = ovr_GetTimeInSeconds() + 10.0f;

    // Initialize FovSideTanMax, which allows us to change all Fov sides at once - Fov
    // starts at default and is clamped to this value.
    FovSideTanLimit = FovPort::Max(Hmd->MaxEyeFov[0], Hmd->MaxEyeFov[1]).GetMaxSideTan();
    FovSideTanMax   = FovPort::Max(Hmd->DefaultEyeFov[0], Hmd->DefaultEyeFov[1]).GetMaxSideTan();

    PositionTrackingEnabled = (Hmd->TrackingCaps & ovrTrackingCap_Position) ? true : false;

	PixelLuminanceOverdrive = (Hmd->DistortionCaps & ovrDistortionCap_Overdrive) ? true : false;

    HqAaDistortion = (Hmd->DistortionCaps & ovrDistortionCap_HqDistortion) ? true : false;

    // *** Configure HMD Stereo settings.
    
    CalculateHmdValues();

    // Query eye height.
    ThePlayer.UserEyeHeight = ovrHmd_GetFloat(Hmd, OVR_KEY_EYE_HEIGHT, ThePlayer.UserEyeHeight);
    ThePlayer.BodyPos.y     = ThePlayer.UserEyeHeight;
    // Center pupil for customization; real game shouldn't need to adjust this.
    CenterPupilDepthMeters  = ovrHmd_GetFloat(Hmd, "CenterPupilDepth", 0.0f);


    ThePlayer.bMotionRelativeToBody = false;  // Default to head-steering for DK1

    if (UsingDebugHmd)
        Menu.SetPopupMessage("NO HMD DETECTED");
    else if (!(ovrHmd_GetTrackingState(Hmd, 0.0f).StatusFlags & ovrStatus_HmdConnected))
        Menu.SetPopupMessage("NO SENSOR DETECTED");
    else if (Hmd->HmdCaps & ovrHmdCap_ExtendDesktop)
        Menu.SetPopupMessage("Press F9 for Full-Screen on Rift");
	else
		Menu.SetPopupMessage("Please put on Rift");

    // Give first message 10 sec timeout, add border lines.
    Menu.SetPopupTimeout(10.0f, true);

    PopulateOptionMenu();

    // *** Identify Scene File & Prepare for Loading

    InitMainFilePath();  
    PopulatePreloadScene();

    LastUpdate = ovr_GetTimeInSeconds();

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-StartPerfLog") && i < argc - 1)
        {
            ovrHmd_StartPerfLog(Hmd, argv[i + 1], 0);
        }
    }

    return 0;
}


bool OculusWorldDemoApp::SetupWindowAndRendering(int argc, const char** argv)
{
    // *** Window creation

	void* windowHandle = pPlatform->SetupWindow(WindowSize.w, WindowSize.h);

	if(!windowHandle)
        return false;
    
	ovrHmd_AttachToWindow( Hmd, windowHandle, NULL, NULL );

    // Report relative mouse motion in OnMouseMove
    pPlatform->SetMouseMode(Mouse_Relative);

    // *** Initialize Rendering

    const char* graphics = "d3d11";  //Default to DX11. Can be overridden below.



    RenderParams.RenderAPIType = ovrRenderAPI_D3D11;

    StringBuffer title;
    title.AppendFormat("VR-Monitors %s : %s", graphics, Hmd->ProductName[0] ? Hmd->ProductName : "<unknown device>");
    pPlatform->SetWindowTitle(title);

    
    // Enable multi-sampling by default.
    RenderParams.Display        = DisplayId(Hmd->DisplayDeviceName, Hmd->DisplayId);
    RenderParams.SrgbBackBuffer = SupportsSrgb && false;   // don't create sRGB back-buffer for OWD
    RenderParams.Multisample    = (SupportsMultisampling && MultisampleEnabled) ? 1 : 0;
    RenderParams.Resolution     = Hmd->Resolution;

    pRender = pPlatform->SetupGraphics(OVR_DEFAULT_RENDER_DEVICE_SET,
                                       graphics, RenderParams); // To do: Remove the graphics argument to SetupGraphics, as RenderParams already has this info.
    return (pRender != nullptr);
}

// Custom formatter for Timewarp interval message.
static String FormatTimewarp(OptionVar* var)
{    
    char    buff[64];
    float   timewarpInterval = *var->AsFloat();
    OVR_sprintf(buff, sizeof(buff), "%.1fms, %.1ffps",
                timewarpInterval * 1000.0f,
                ( timewarpInterval > 0.000001f ) ? 1.0f / timewarpInterval : 10000.0f);
    return String(buff);
}

static String FormatMaxFromSideTan(OptionVar* var)
{
    char   buff[64];
    float  degrees = 2.0f * atan(*var->AsFloat()) * (180.0f / MATH_FLOAT_PI);
    OVR_sprintf(buff, sizeof(buff), "%.1f Degrees", degrees);
    return String(buff);
}

void OculusWorldDemoApp::PopulateOptionMenu()
{
    // For shortened function member access.
    typedef OculusWorldDemoApp OWD;


    // *** Scene Content Sub-Menu
    if (SupportsMultisampling)
    Menu.AddBool("Render Target.MultiSample 'F4'",    &MultisampleEnabled)    .AddShortcutKey(Key_F4).SetNotify(this, &OWD::MultisampleChange);


    Menu.AddBool( "Timewarp.ComputeShaderEnabled",  &ComputeShaderEnabled).
    																SetNotify(this, &OWD::HmdSettingChange);

    Menu.AddBool("VSync 'V'",           &VsyncEnabled)          .AddShortcutKey(Key_V).SetNotify(this, &OWD::HmdSettingChange);
    Menu.AddTrigger("Recenter HMD pose 'R'").AddShortcutKey(Key_R).SetNotify(this, &OWD::ResetHmdPose);

    if (!(Hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
    {
        Menu.AddBool("Mirror To Window", &MirrorToWindow).
                                         AddShortcutKey(Key_M).SetNotify(this, &OWD::MirrorSettingChange);
    }
}


void OculusWorldDemoApp::CalculateHmdValues()
{
	// Initialize eye rendering information for ovrHmd_Configure.
	// The viewport sizes are re-computed in case RenderTargetSize changed due to HW limitations.
	ovrFovPort eyeFov[2];
	eyeFov[0] = Hmd->DefaultEyeFov[0];
	eyeFov[1] = Hmd->DefaultEyeFov[1];

	// Clamp Fov based on our dynamically adjustable FovSideTanMax.
	// Most apps should use the default, but reducing Fov does reduce rendering cost.
	eyeFov[0] = FovPort::Min(eyeFov[0], FovPort(FovSideTanMax));
	eyeFov[1] = FovPort::Min(eyeFov[1], FovPort(FovSideTanMax));


	// Configure Stereo settings. Default pixel density is 1.0f.
	Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(Hmd, ovrEye_Left, eyeFov[0], DesiredPixelDensity);
	Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(Hmd, ovrEye_Right, eyeFov[1], DesiredPixelDensity);

		Sizei tex0Size = EnsureRendertargetAtLeastThisBig(Rendertarget_Left, recommenedTex0Size);
		Sizei tex1Size = EnsureRendertargetAtLeastThisBig(Rendertarget_Right, recommenedTex1Size);

		EyeRenderSize[0] = Sizei::Min(tex0Size, recommenedTex0Size);
		EyeRenderSize[1] = Sizei::Min(tex1Size, recommenedTex1Size);

		// Store texture pointers and viewports that will be passed for rendering.
		EyeTexture[0] = RenderTargets[Rendertarget_Left].OvrTex;
		EyeTexture[0].Header.TextureSize = tex0Size;
		EyeTexture[0].Header.RenderViewport = Recti(EyeRenderSize[0]);
		EyeTexture[1] = RenderTargets[Rendertarget_Right].OvrTex;
		EyeTexture[1].Header.TextureSize = tex1Size;
		EyeTexture[1].Header.RenderViewport = Recti(EyeRenderSize[1]);


	DrawEyeTargets = (MultisampleEnabled && SupportsMultisampling) ? MsaaRenderTargets : RenderTargets;

	// Hmd caps.
	unsigned hmdCaps = (VsyncEnabled ? 0 : ovrHmdCap_NoVSync);
	if (IsLowPersistence)
		hmdCaps |= ovrHmdCap_LowPersistence;

	// ovrHmdCap_DynamicPrediction - enables internal latency feedback
	if (DynamicPrediction)
		hmdCaps |= ovrHmdCap_DynamicPrediction;

	if (!MirrorToWindow)
		hmdCaps |= ovrHmdCap_NoMirrorToWindow;

	// If using our driver, display status overlay messages.
	if (!(Hmd->HmdCaps & ovrHmdCap_ExtendDesktop) && (NotificationTimeout != 0.0f))
	{
		GetPlatformCore()->SetNotificationOverlay(0, 28, 8,
			"Rendering to the Hmd - Please put on your Rift");
		GetPlatformCore()->SetNotificationOverlay(1, 24, -8,
			MirrorToWindow ? "'M' - Mirror to Window [On]" : "'M' - Mirror to Window [Off]");
	}


	ovrHmd_SetEnabledCaps(Hmd, hmdCaps);


	ovrRenderAPIConfig config = pRender->Get_ovrRenderAPIConfig();
	unsigned           distortionCaps = ovrDistortionCap_Chromatic;

	if (FadedBorder)
		distortionCaps |= ovrDistortionCap_Vignette;
	if (SupportsSrgb)
		distortionCaps |= ovrDistortionCap_SRGB;
	if (PixelLuminanceOverdrive)
		distortionCaps |= ovrDistortionCap_Overdrive;
	if (TimewarpEnabled)
		distortionCaps |= ovrDistortionCap_TimeWarp;
	if (HqAaDistortion)
		distortionCaps |= ovrDistortionCap_HqDistortion;

	if (ComputeShaderEnabled)
		distortionCaps |= ovrDistortionCap_ComputeShader;

	if (!ovrHmd_ConfigureRendering(Hmd, &config, distortionCaps, eyeFov, EyeRenderDesc))
	{
		// Fail exit? TBD
		return;
	}

	unsigned sensorCaps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection;
	if (PositionTrackingEnabled)
		sensorCaps |= ovrTrackingCap_Position;

	if (StartTrackingCaps != sensorCaps)
	{
		ovrHmd_ConfigureTracking(Hmd, sensorCaps, 0);
		StartTrackingCaps = sensorCaps;
	}

	// Calculate projections
	Projection[0] = ovrMatrix4f_Projection(EyeRenderDesc[0].Fov, 0.01f, 10000.0f, true);
	Projection[1] = ovrMatrix4f_Projection(EyeRenderDesc[1].Fov, 0.01f, 10000.0f, true);

	float    orthoDistance = 0.8f; // 2D is 0.8 meter from camera
	Vector2f orthoScale0 = Vector2f(1.0f) / Vector2f(EyeRenderDesc[0].PixelsPerTanAngleAtCenter);
	Vector2f orthoScale1 = Vector2f(1.0f) / Vector2f(EyeRenderDesc[1].PixelsPerTanAngleAtCenter);

	OrthoProjection[0] = ovrMatrix4f_OrthoSubProjection(Projection[0], orthoScale0, orthoDistance,
		EyeRenderDesc[0].HmdToEyeViewOffset.x);
	OrthoProjection[1] = ovrMatrix4f_OrthoSubProjection(Projection[1], orthoScale1, orthoDistance,
		EyeRenderDesc[1].HmdToEyeViewOffset.x);

	// all done
	HmdSettingsChanged = false;
}



// Returns the actual size present.
Sizei OculusWorldDemoApp::EnsureRendertargetAtLeastThisBig(int rtNum, Sizei requestedSize)
{
    OVR_ASSERT((rtNum >= 0) && (rtNum < Rendertarget_LAST));

    // Texture size that we already have might be big enough.
    Sizei newRTSize;

    RenderTarget& rt = RenderTargets[rtNum];
    RenderTarget& msrt = MsaaRenderTargets[rtNum];
    if (!rt.pTex)
    {
        // Hmmm... someone nuked my texture. Rez change or similar. Make sure we reallocate.
        rt.OvrTex.Header.TextureSize = Sizei(0);
        
        if(MultisampleEnabled && SupportsMultisampling)
            msrt.OvrTex.Header.TextureSize = Sizei(0);

        newRTSize = requestedSize;
    }
    else
    {
        newRTSize = rt.OvrTex.Header.TextureSize;
    }

    // %50 linear growth each time is a nice balance between being too greedy
    // for a 2D surface and too slow to prevent fragmentation.
    while ( newRTSize.w < requestedSize.w )
    {
        newRTSize.w += newRTSize.w/2;
    }
    while ( newRTSize.h < requestedSize.h )
    {
        newRTSize.h += newRTSize.h/2;
    }

    // Put some sane limits on it. 4k x 4k is fine for most modern video cards.
    // Nobody should be messing around with surfaces smaller than 4k pixels these days.
    newRTSize = Sizei::Max(Sizei::Min(newRTSize, Sizei(4096)), Sizei(64));

    // Does that require actual reallocation?
    if (Sizei(rt.OvrTex.Header.TextureSize) != newRTSize)        
    {        
        int format = Texture_RGBA | Texture_RenderTarget;
        if (SupportsSrgb)
            format |= Texture_SRGB;

        rt.pTex = *pRender->CreateTexture(format, newRTSize.w, newRTSize.h, NULL);
        rt.pTex->SetSampleMode(Sample_ClampBorder | Sample_Linear);
        
        // Configure texture for SDK Rendering.
        rt.OvrTex = rt.pTex->Get_ovrTexture();
        
        if(MultisampleEnabled && SupportsMultisampling)
        {
            int msaaformat = format | 4;    // 4 is MSAA rate

            msrt.pTex = *pRender->CreateTexture(msaaformat, newRTSize.w, newRTSize.h, NULL);
            msrt.pTex->SetSampleMode(Sample_ClampBorder | Sample_Linear);

            // Configure texture for SDK Rendering.
            msrt.OvrTex = rt.pTex->Get_ovrTexture();
        }
    }
    
    return newRTSize;
}


//-----------------------------------------------------------------------------
// ***** Message Handlers

void OculusWorldDemoApp::OnResize(int width, int height)
{
    WindowSize = Sizei(width, height);
    HmdSettingsChanged = true;
}

void OculusWorldDemoApp::OnMouseMove(int x, int y, int modifiers)
{
    OVR_UNUSED(y);
    if(modifiers & Mod_MouseRelative)
    {
        // Get Delta
        int dx = x;

        // Apply to rotation. Subtract for right body frame rotation,
        // since yaw rotation is positive CCW when looking down on XZ plane.
        ThePlayer.BodyYaw   -= (Sensitivity * dx) / 360.0f;
    }
}


void OculusWorldDemoApp::OnKey(OVR::KeyCode key, int chr, bool down, int modifiers)
{
    if (down)
    {   // Dismiss Safety warning with any key.
        ovrHmd_DismissHSWDisplay(Hmd);
    }

    if (Menu.OnKey(key, chr, down, modifiers))
        return;

    // Handle player movement keys.
    if (ThePlayer.HandleMoveKey(key, down))
        return;

    switch(key)
    {
    case Key_Q:
        if (down && (modifiers & Mod_Control))
            pPlatform->Exit(0);
        break;
        
    case Key_Escape:
        // Back to primary windowed
        if (!down) ChangeDisplay ( true, false, false );
        break;
        
        
	case Key_Space:
        if (!down)
        {
            TextScreen = (enum TextScreen)((TextScreen + 1) % Text_Count);
        }
        break;

    case Key_Shift:
        ShiftDown = down;
        break;
    case Key_Control:
        CtrlDown = down;
        break;

    case Key_Num1:
        ThePlayer.BodyPos = Vector3f(-1.85f, 6.0f, -0.52f);
        ThePlayer.BodyPos.y += ThePlayer.UserEyeHeight;
        ThePlayer.BodyYaw = 3.1415f / 2;
        ThePlayer.HandleMovement(0, &CollisionModels, &GroundCollisionModels, ShiftDown);
        break;

     default:
        break;
    }
}

//-----------------------------------------------------------------------------


Matrix4f OculusWorldDemoApp::CalculateViewFromPose(const Posef& pose)
{
    Posef worldPose = ThePlayer.VirtualWorldTransformfromRealPose(pose);

    // Rotate and position View Camera
    Vector3f up      = worldPose.Rotation.Rotate(UpVector);
    Vector3f forward = worldPose.Rotation.Rotate(ForwardVector);

    // Transform the position of the center eye in the real world (i.e. sitting in your chair)
    // into the frame of the player's virtual body.

    Vector3f viewPos = worldPose.Translation;

    Matrix4f view = Matrix4f::LookAtRH(viewPos, viewPos + forward, up);
    return view;
}



void OculusWorldDemoApp::OnIdle()
{
    double curtime = ovr_GetTimeInSeconds();
    // If running slower than 10fps, clamp. Helps when debugging, because then dt can be minutes!
    float  dt      = Alg::Min<float>(float(curtime - LastUpdate), 0.1f);
    LastUpdate     = curtime;    


    Profiler.RecordSample(RenderProfiler::Sample_FrameStart);

    if (LoadingState == LoadingState_DoLoad)
    {
        PopulateScene(MainFilePath.ToCStr());
        LoadingState = LoadingState_Finished;
        return;
    }    

    if (HmdSettingsChanged)
    {
        CalculateHmdValues();        
    }
    
    // Kill overlays in non-mirror mode after timeout.
    if ((NotificationTimeout != 0.0) && (curtime > NotificationTimeout))
    {
        if (MirrorToWindow)
        {
            GetPlatformCore()->SetNotificationOverlay(0,0,0,0);
            GetPlatformCore()->SetNotificationOverlay(1,0,0,0);
        }
        NotificationTimeout = 0.0;
    }


    HmdFrameTiming = ovrHmd_BeginFrame(Hmd, 0);


    ovrTrackingState trackState = ovrHmd_GetTrackingState(Hmd, HmdFrameTiming.ScanoutMidpointSeconds);
    HmdStatus = trackState.StatusFlags;

    // Report vision tracking
	bool hadVisionTracking = HaveVisionTracking;
	HaveVisionTracking = (trackState.StatusFlags & ovrStatus_PositionTracked) != 0;
	if (HaveVisionTracking && !hadVisionTracking)
		Menu.SetPopupMessage("Vision Tracking Acquired");
    if (!HaveVisionTracking && hadVisionTracking)
		Menu.SetPopupMessage("Lost Vision Tracking");

    // Report position tracker
    bool hadPositionTracker = HavePositionTracker;
    HavePositionTracker = (trackState.StatusFlags & ovrStatus_PositionConnected) != 0;
    if (HavePositionTracker && !hadPositionTracker)
        Menu.SetPopupMessage("Position Tracker Connected");
    if (!HavePositionTracker && hadPositionTracker)
        Menu.SetPopupMessage("Position Tracker Disconnected");

    // Report position tracker
    bool hadHMDConnected = HaveHMDConnected;
    HaveHMDConnected = (trackState.StatusFlags & ovrStatus_HmdConnected) != 0;
    if (HaveHMDConnected && !hadHMDConnected)
        Menu.SetPopupMessage("HMD Connected");
    if (!HaveHMDConnected && hadHMDConnected)
        Menu.SetPopupMessage("HMD Disconnected");

    UpdateVisionProcessingTime(trackState);

    // Check if any new devices were connected.
    ProcessDeviceNotificationQueue();
    // FPS count and timing.
    UpdateFrameRateCounter(curtime);

    
    // Update pose based on frame!
    ThePlayer.HeadPose = trackState.HeadPose.ThePose;
    // Movement/rotation with the gamepad.
    ThePlayer.BodyYaw -= ThePlayer.GamepadRotate.x * dt;
    ThePlayer.HandleMovement(dt, &CollisionModels, &GroundCollisionModels, ShiftDown);


    // Record after processing time.
    Profiler.RecordSample(RenderProfiler::Sample_AfterGameProcessing);    


    // Determine if we are rendering this frame. Frame rendering may be
    // skipped based on FreezeEyeUpdate and Time-warp timing state.
    bool bupdateRenderedView = FrameNeedsRendering(curtime);
    
    if (bupdateRenderedView)
    {
        // If render texture size is changing, apply dynamic changes to viewport.
        ApplyDynamicResolutionScaling();

        pRender->BeginScene(PostProcess_None);

        ovrTrackingState hmdState;
        ovrVector3f hmdToEyeViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset, EyeRenderDesc[1].HmdToEyeViewOffset };
        ovrHmd_GetEyePoses(Hmd, 0, hmdToEyeViewOffset, EyeRenderPose, &hmdState);

        // It is important to have head movement in scale with IPD.
        // If you shrink one, you should also shrink the other.
        // So with zero IPD (i.e. everything at infinity),
        // head movement should also be zero.
        EyeRenderPose[0].Position = ((Vector3f)EyeRenderPose[0].Position) * PositionTrackingScale;
        EyeRenderPose[1].Position = ((Vector3f)EyeRenderPose[1].Position) * PositionTrackingScale;


            // Separate eye rendering - each eye gets its own render target.
		for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
		{
			ovrEyeType eye = Hmd->EyeRenderOrder[eyeIndex];
			pRender->SetRenderTarget(
				DrawEyeTargets[(eye == 0) ? Rendertarget_Left : Rendertarget_Right].pTex);
			pRender->Clear();

			View = CalculateViewFromPose(EyeRenderPose[eye]);
			RenderEyeView(eye);
		}

        pRender->SetDefaultRenderTarget();
        pRender->FinishScene();

		if (MultisampleEnabled && SupportsMultisampling)
		{

			for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
				pRender->ResolveMsaa(MsaaRenderTargets[eyeIndex].pTex, RenderTargets[eyeIndex].pTex);

		}
    }
       
    Profiler.RecordSample(RenderProfiler::Sample_AfterEyeRender);

    // TODO: These happen inside ovrHmd_EndFrame; need to hook into it.
    //Profiler.RecordSample(RenderProfiler::Sample_BeforeDistortion);
    ovrHmd_EndFrame(Hmd, EyeRenderPose, EyeTexture);
    Profiler.RecordSample(RenderProfiler::Sample_AfterPresent);    
}



// Determine whether this frame needs rendering based on time-warp timing and flags.
bool OculusWorldDemoApp::FrameNeedsRendering(double curtime)
{
	static double   lastUpdate = 0.0;
	double          renderInterval = TimewarpRenderIntervalInSeconds;
	double          timeSinceLast = curtime - lastUpdate;
	bool            updateRenderedView = true;


	if ((timeSinceLast < 0.0) || ((float)timeSinceLast > renderInterval))
	{
		// This allows us to do "fractional" speeds, e.g. 45fps rendering on a 60fps display.
		lastUpdate += renderInterval;
		if (timeSinceLast > 5.0)
		{
			// renderInterval is probably tiny (i.e. "as fast as possible")
			lastUpdate = curtime;
		}

		updateRenderedView = true;
	}
	else
	{
		updateRenderedView = false;
	}


	return updateRenderedView;
}


void OculusWorldDemoApp::ApplyDynamicResolutionScaling()
{
   
    // Demonstrate dynamic-resolution rendering.
    // This demo is too simple to actually have a framerate that varies that much, so we'll
    // just pretend this is trying to cope with highly dynamic rendering load.
    float dynamicRezScale = 1.0f;

    {
        // Hacky stuff to make up a scaling...
        // This produces value oscillating as follows: 0 -> 1 -> 0.        
        static double dynamicRezStartTime   = ovr_GetTimeInSeconds();
        float         dynamicRezPhase       = float ( ovr_GetTimeInSeconds() - dynamicRezStartTime );
        const float   dynamicRezTimeScale   = 4.0f;

        dynamicRezPhase /= dynamicRezTimeScale;
        if ( dynamicRezPhase < 1.0f )
        {
            dynamicRezScale = dynamicRezPhase;
        }
        else if ( dynamicRezPhase < 2.0f )
        {
            dynamicRezScale = 2.0f - dynamicRezPhase;
        }
        else
        {
            // Reset it to prevent creep.
            dynamicRezStartTime = ovr_GetTimeInSeconds();
            dynamicRezScale     = 0.0f;
        }

        // Map oscillation: 0.5 -> 1.0 -> 0.5
        dynamicRezScale = dynamicRezScale * 0.5f + 0.5f;
    }

    Sizei sizeLeft  = EyeRenderSize[0];
    Sizei sizeRight = EyeRenderSize[1];
    
    // This viewport is used for rendering and passed into ovrHmd_EndEyeRender.
    EyeTexture[0].Header.RenderViewport.Size = Sizei(int(sizeLeft.w  * dynamicRezScale),
                                                     int(sizeLeft.h  * dynamicRezScale));
    EyeTexture[1].Header.RenderViewport.Size = Sizei(int(sizeRight.w * dynamicRezScale),
                                                     int(sizeRight.h * dynamicRezScale));
}


void OculusWorldDemoApp::UpdateFrameRateCounter(double curtime)
{
    FrameCounter++;
	TotalFrameCounter++;
    float secondsSinceLastMeasurement = (float)( curtime - LastFpsUpdate );

    if (secondsSinceLastMeasurement >= SecondsOfFpsMeasurement)
    {
        SecondsPerFrame = (float)( curtime - LastFpsUpdate ) / (float)FrameCounter;
        FPS             = 1.0f / SecondsPerFrame;
        LastFpsUpdate   = curtime;
        FrameCounter =   0;
    }
}

void OculusWorldDemoApp::UpdateVisionProcessingTime(const ovrTrackingState& trackState)
{
    // Update LastVisionProcessingTime
    if (trackState.LastVisionProcessingTime != LastVisionProcessingTime)
    {
        LastVisionProcessingTime = trackState.LastVisionProcessingTime;

        VisionProcessingSum += LastVisionProcessingTime;

        if (VisionTimesCount >= 20)
        {
            VisionProcessingAverage = VisionProcessingSum / 20.;
            VisionProcessingSum = 0.;
            VisionTimesCount = 0;
        }
        else
        {
            VisionTimesCount++;
        }
    }
}

void OculusWorldDemoApp::RenderEyeView(ovrEyeType eye)
{
    Recti    renderViewport = EyeTexture[eye].Header.RenderViewport;

    // *** 3D - Configures Viewport/Projection and Render
    
    pRender->ApplyStereoParams(renderViewport, Projection[eye]);
    pRender->SetDepthMode(true, true);

    Matrix4f baseTranslate = Matrix4f::Translation(ThePlayer.BodyPos);
    Matrix4f baseYaw       = Matrix4f::RotationY(ThePlayer.BodyYaw.Get());


	MainScene.Render(pRender, View);        


    // *** 2D Text - Configure Orthographic rendering.

    // Render UI in 2D orthographic coordinate system that maps [-1,1] range
    // to a readable FOV area centered at your eye and properly adjusted.
    pRender->ApplyStereoParams(renderViewport, OrthoProjection[eye]);
    pRender->SetDepthMode(false, false);

    // We set this scale up in CreateOrthoSubProjection().
    float textHeight = 22.0f;

    // Display Loading screen-shot in frame 0.
    if (LoadingState != LoadingState_Finished)
    {
        const float scale = textHeight * 25.0f;
        Matrix4f view ( scale, 0.0f, 0.0f, 0.0f, scale, 0.0f, 0.0f, 0.0f, scale );
        LoadingScene.Render(pRender, view);
        String loadMessage = String("Loading ") + MainFilePath;
        DrawTextBox(pRender, 0.0f, -textHeight, textHeight, loadMessage.ToCStr(), DrawText_HCenter);
        LoadingState = LoadingState_DoLoad;
    }

    // HUD overlay brought up by spacebar.
    RenderTextInfoHud(textHeight);

    // Menu brought up by 
    Menu.Render(pRender);
}



// NOTE - try to keep these in sync with the PDF docs!
static const char* HelpText1 =
    "Spacebar 	            \t500 Toggle debug info overlay\n"
    "W, S            	    \t500 Move forward, back\n"
    "A, D 		    	    \t500 Strafe left, right\n"
    "Mouse move 	        \t500 Look left, right\n"
    "Left gamepad stick     \t500 Move\n"
    "Right gamepad stick    \t500 Turn\n"
    "T			            \t500 Reset player position";
    
static const char* HelpText2 =        
    "R              \t250 Reset sensor orientation\n"
    "Esc            \t250 Cancel full-screen\n"
    "F4			    \t250 Multisampling toggle\n"    
    "Ctrl+Q		    \t250 Quit";


void FormatLatencyReading(char* buff, size_t size, float val)
{    
    if (val < 0.000001f)
        OVR_strcpy(buff, size, "N/A   ");
    else
        OVR_sprintf(buff, size, "%4.2fms", val * 1000.0f);    
}


void OculusWorldDemoApp::RenderTextInfoHud(float textHeight)
{
    // View port & 2D ortho projection must be set before call.
    
    float hmdYaw, hmdPitch, hmdRoll;
    switch(TextScreen)
    {
    case Text_Info:
    {
        char buf[512];

        // Average FOVs.
        FovPort leftFov  = EyeRenderDesc[0].Fov;
        FovPort rightFov = EyeRenderDesc[1].Fov;
        
        // Rendered size changes based on selected options & dynamic rendering.
        int pixelSizeWidth = EyeTexture[0].Header.RenderViewport.Size.w +
                               EyeTexture[1].Header.RenderViewport.Size.w;
        int pixelSizeHeight = ( EyeTexture[0].Header.RenderViewport.Size.h +
                                EyeTexture[1].Header.RenderViewport.Size.h ) / 2;

        // No DK2, no message.
        char latency2Text[128] = "";
        {
            //float latency2 = ovrHmd_GetMeasuredLatencyTest2(Hmd) * 1000.0f; // show it in ms
            //if (latency2 > 0)
            //    OVR_sprintf(latency2Text, sizeof(latency2Text), "%.2fms", latency2);

            float latencies[3] = { 0.0f, 0.0f, 0.0f };
            if (ovrHmd_GetFloatArray(Hmd, "DK2Latency", latencies, 3) == 3)
            {
                char latencyText0[32], latencyText1[32], latencyText2[32];
                FormatLatencyReading(latencyText0, sizeof(latencyText0), latencies[0]);
                FormatLatencyReading(latencyText1, sizeof(latencyText1), latencies[1]);
                FormatLatencyReading(latencyText2, sizeof(latencyText2), latencies[2]);

                OVR_sprintf(latency2Text, sizeof(latency2Text),
                            " DK2 Latency  Ren: %s  TWrp: %s\n"
                            " PostPresent: %s  VisionProc: %1.2f ms ",
                            latencyText0, latencyText1, latencyText2,
                            (float)VisionProcessingAverage * 1000);
            }
        }

        ThePlayer.HeadPose.Rotation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&hmdYaw, &hmdPitch, &hmdRoll);
        OVR_sprintf(buf, sizeof(buf),
                    " HMD YPR:%4.0f %4.0f %4.0f   Player Yaw: %4.0f\n"
                    " FPS: %.1f  ms/frame: %.1f Frame: %03d %d\n"
                    " Pos: %3.2f, %3.2f, %3.2f  HMD: %s\n"
                    " EyeHeight: %3.2f, IPD: %3.1fmm\n" //", Lens: %s\n"
                    " FOV %3.1fx%3.1f, Resolution: %ix%i\n"
                    "%s",
                    RadToDegree(hmdYaw), RadToDegree(hmdPitch), RadToDegree(hmdRoll),
                    RadToDegree(ThePlayer.BodyYaw.Get()),
                    FPS, SecondsPerFrame * 1000.0f, FrameCounter, TotalFrameCounter % 2,
                    ThePlayer.BodyPos.x, ThePlayer.BodyPos.y, ThePlayer.BodyPos.z,
                    //GetDebugNameHmdType ( TheHmdRenderInfo.HmdType ),
                    Hmd->ProductName,
                    ThePlayer.UserEyeHeight,
                    ovrHmd_GetFloat(Hmd, OVR_KEY_IPD, 0) * 1000.0f,
                    //( EyeOffsetFromNoseLeft + EyeOffsetFromNoseRight ) * 1000.0f,
                    //GetDebugNameEyeCupType ( TheHmdRenderInfo.EyeCups ),  // Lens/EyeCup not exposed
                    
                    (leftFov.GetHorizontalFovDegrees() + rightFov.GetHorizontalFovDegrees()) * 0.5f,
                    (leftFov.GetVerticalFovDegrees() + rightFov.GetVerticalFovDegrees()) * 0.5f,

                    pixelSizeWidth, pixelSizeHeight,

                    latency2Text
                    );

#if 0   // Enable if interested in texture memory usage stats
        size_t texMemInMB = pRender->GetTotalTextureMemoryUsage() / (1024 * 1024); // 1 MB
        if (texMemInMB)
        {
            char gpustat[256];
            OVR_sprintf(gpustat, sizeof(gpustat), "\nGPU Tex: %u MB", texMemInMB);
            OVR_strcat(buf, sizeof(buf), gpustat);
        }
#endif

        DrawTextBox(pRender, 0.0f, 0.0f, textHeight, buf, DrawText_Center);
    }
    break;
    
    case Text_Timing:    
        Profiler.DrawOverlay(pRender);    
    break;
    
    case Text_Help1:
        DrawTextBox(pRender, 0.0f, 0.0f, textHeight, HelpText1, DrawText_Center);
        break;
    case Text_Help2:
        DrawTextBox(pRender, 0.0f, 0.0f, textHeight, HelpText2, DrawText_Center);
        break;
    
    case Text_None:
        break;

    default:
        OVR_ASSERT ( !"Missing text screen" );
        break;    
    }
}


//-----------------------------------------------------------------------------
// ***** Callbacks For Menu changes

// Non-trivial callback go here.

void OculusWorldDemoApp::HmdSensorToggle(OptionVar* var)
{
	if (*var->AsBool())
	{
		EnableSensor = true;
		if (!ovrHmd_ConfigureTracking(Hmd, StartTrackingCaps, 0))
		{
			OVR_ASSERT(false);
		}
	}
	else
	{
		EnableSensor = false;
		ovrHmd_ConfigureTracking(Hmd, 0, 0);
	}
}

void OculusWorldDemoApp::HmdSettingChangeFreeRTs(OptionVar*)
{
    HmdSettingsChanged = true;
    // Cause the RTs to be recreated with the new mode.
    for ( int rtNum = 0; rtNum < Rendertarget_LAST; rtNum++ )
    {
        RenderTargets[rtNum].pTex = NULL;
        MsaaRenderTargets[rtNum].pTex = NULL;
    }
}

void OculusWorldDemoApp::MultisampleChange(OptionVar*)
{
    HmdSettingChangeFreeRTs();
}

void OculusWorldDemoApp::ResetHmdPose(OptionVar* /* = 0 */)
{
    ovrHmd_RecenterPose(Hmd);
    Menu.SetPopupMessage("Sensor Fusion Recenter Pose");
}

//-----------------------------------------------------------------------------

void OculusWorldDemoApp::ProcessDeviceNotificationQueue()
{
    // TBD: Process device plug & Unplug     
}

//-----------------------------------------------------------------------------
void OculusWorldDemoApp::ChangeDisplay ( bool bBackToWindowed, bool bNextFullscreen,
                                         bool bFullWindowDebugging )
{
    // Display mode switching doesn't make sense in App driver mode.
    if (!(Hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
        return;

    // Exactly one should be set...
    OVR_ASSERT ( ( bBackToWindowed ? 1 : 0 ) + ( bNextFullscreen ? 1 : 0 ) +
                 ( bFullWindowDebugging ? 1 : 0 ) == 1 );
    OVR_UNUSED ( bNextFullscreen );

    if ( bFullWindowDebugging )
    {
        // Slightly hacky. Doesn't actually go fullscreen, just makes a screen-sized wndow.
        // This has higher latency than fullscreen, and is not intended for actual use, 
        // but makes for much nicer debugging on some systems.
        RenderParams = pRender->GetParams();
        RenderParams.Display = DisplayId(Hmd->DisplayDeviceName, Hmd->DisplayId);
        pRender->SetParams(RenderParams);

        pPlatform->SetMouseMode(Mouse_Normal);            
        pPlatform->SetFullscreen(RenderParams, pRender->IsFullscreen() ? Display_Window : Display_FakeFullscreen);
        pPlatform->SetMouseMode(Mouse_Relative); // Avoid mode world rotation jump.
   
        // If using an HMD, enable post-process (for distortion) and stereo.        
        if (RenderParams.IsDisplaySet() && pRender->IsFullscreen())
        {            
            //SetPostProcessingMode ( PostProcess );
        }
    }
    else
    {
        int screenCount = pPlatform->GetDisplayCount();

        int screenNumberToSwitchTo;
        if ( bBackToWindowed )
        {
            screenNumberToSwitchTo = -1;
        }
        else
        {
            if (!pRender->IsFullscreen())
            {
                // Currently windowed.
                // Try to find HMD Screen, making it the first screen in the full-screen cycle.
                FirstScreenInCycle = 0;
                if (!UsingDebugHmd)
                {
                    DisplayId HMD (Hmd->DisplayDeviceName, Hmd->DisplayId);
                    for (int i = 0; i< screenCount; i++)
                    {   
                        if (pPlatform->GetDisplay(i) == HMD)
                        {
                            FirstScreenInCycle = i;
                            break;
                        }
                    }            
                }
                ScreenNumber = FirstScreenInCycle;
                screenNumberToSwitchTo = ScreenNumber;
            }
            else
            {
                // Currently fullscreen, so cycle to the next screen.
                ScreenNumber++;
                if (ScreenNumber == screenCount)
                {
                    ScreenNumber = 0;
                }
                screenNumberToSwitchTo = ScreenNumber;
                if (ScreenNumber == FirstScreenInCycle)
                {
                    // We have cycled through all the fullscreen displays, so go back to windowed mode.
                    screenNumberToSwitchTo = -1;
                }
            }
        }

        pPlatform->SetFullscreen(RenderParams, Display_Window);
        if ( screenNumberToSwitchTo >= 0 )
        {
            // Go fullscreen.
            RenderParams.Display = pPlatform->GetDisplay(screenNumberToSwitchTo);
            pRender->SetParams(RenderParams);
            pPlatform->SetFullscreen(RenderParams, Display_Fullscreen);            
            
        }
    }

    
    // Updates render target pointers & sizes.
    HmdSettingChangeFreeRTs();    
}


int OculusWorldDemoApp::HandleException(uintptr_t /*userValue*/, OVR::ExceptionHandler* /*pExceptionHandler*/, ExceptionInfo* /*pExceptionInfo*/, const char* reportFilePath)
{
    const char* uiText = ExceptionHandler::GetExceptionUIText(reportFilePath);

    if(uiText)
    {
        OVR::Util::DisplayMessageBox("Exception encountered in OculusWorldDemo", uiText);
        ExceptionHandler::FreeExceptionUIText(uiText);
    }

    return 0;
}





//-------------------------------------------------------------------------------------

OVR_PLATFORM_APP(OculusWorldDemoApp);
