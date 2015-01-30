#include "VR-MonitorsApp.h"

// what happens in the constructer, must initialize the variables before it starts
VRMonitorsApp::VRMonitorsApp() :
pRender(0),
RenderParams(),
WindowSize(1280, 800), 
Hmd(0)
{}


VRMonitorsApp::~VRMonitorsApp()
{
	CleanupDrawTextFont();

	if (Hmd)
	{
		ovrHmd_Destroy(Hmd);
		Hmd = 0;
	}

	ovr_Shutdown();
}

// when the app starts it'll call this function because of the platform file to initialize everything for the oculus

int VRMonitorsApp::OnStartup(int argc, const char** argv) {
	
	// name the thread so whenever we access it, we can use this thread name
	OVR::Thread::SetCurrentThreadName("OWDMain");

	// Get everything organized for displaying onto the oculus

// if the user is using windows 32 bit we need to set the thread to the highest priority
#if defined(OVR_OS_WIN32)
	OVR::Thread::SetCurrentPriority(Thread::HighestPriority);

	if (OVR::Thread::GetCPUCount() >= 4) // Don't do this unless there are at least 4 processors, otherwise the process could hog the machine.
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	}
#endif

	// initialize the ovr library
	ovr_Initialize();

	// create a connection to the oculus from the library
	Hmd = ovrHmd_Create(0);

	// if it doesn't detect and oculus then we need to simulate one
	if (!Hmd)
	{
		Hmd = ovrHmd_CreateDebug(ovrHmd_DK2);

		// check it the debug one was created
		if (!Hmd)
		{   // if not then exit with 1 so we know something went wrong
			return 1;
		}
	}

	// we know the oculus is using extended desktop so the size of our window is the size of the oculus resolution
	if (Hmd->HmdCaps & ovrHmdCap_ExtendDesktop)
	{
		WindowSize = Hmd->Resolution;
	}
	// otherwise it's what we create it as
	else
	{
		WindowSize = Sizei(1100, 618);
	}


	// create the window to start displaying everything

	if (!SetupWindowAndRendering(argc, argv))
	{
		// if it didn't work then return 1 to say there was an error
		return 1;
	}

	// everything went smooth so exit fine
	return 0;
}


bool VRMonitorsApp::SetupWindowAndRendering(int argc, const char** argv)
{
	// *** Window creation

	void* windowHandle = pPlatform->SetupWindow(WindowSize.w, WindowSize.h);

	if (!windowHandle)
		return false;

	ovrHmd_AttachToWindow(Hmd, windowHandle, NULL, NULL);

	// Report relative mouse motion in OnMouseMove
	pPlatform->SetMouseMode(Mouse_Relative);

	// *** Initialize Rendering

	const char* graphics = "d3d11";  //Default to DX11. Can be overridden below.

	RenderParams.RenderAPIType = ovrRenderAPI_D3D11;

	StringBuffer title;
	title.AppendFormat("VR-Monitors %s : %s", graphics, Hmd->ProductName[0] ? Hmd->ProductName : "<unknown device>");
	pPlatform->SetWindowTitle(title);


	// Enable multi-sampling by default.
	RenderParams.Display = DisplayId(Hmd->DisplayDeviceName, Hmd->DisplayId);
	RenderParams.SrgbBackBuffer = true;
	RenderParams.Multisample = true;
	RenderParams.Resolution = Hmd->Resolution;

	pRender = pPlatform->SetupGraphics(OVR_DEFAULT_RENDER_DEVICE_SET,
		graphics, RenderParams); // To do: Remove the graphics argument to SetupGraphics, as RenderParams already has this info.
	return (pRender != nullptr);
}

void VRMonitorsApp::OnKey(OVR::KeyCode key, int chr, bool down, int modifiers)
{
	if (down)
	{   // Dismiss Safety warning with any key.
		ovrHmd_DismissHSWDisplay(Hmd);
	}

	switch (key)
	{
	case Key_Q:
		if (down && (modifiers & Mod_Control))
			pPlatform->Exit(0);
		break;

	default:
		break;
	}
}

OVR_PLATFORM_APP(VRMonitorsApp);