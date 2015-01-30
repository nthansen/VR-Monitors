#include "VR-MonitorsApp.h"

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