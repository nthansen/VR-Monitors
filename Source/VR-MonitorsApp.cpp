#include "VR-MonitorsApp.h"

int VRMonitorsApp::OnStartup(int argc, const char** argv) {
	OVR::Thread::SetCurrentThreadName("OWDMain");

	// *** Oculus HMD & Sensor Initialization

	// Create DeviceManager and first available HMDDevice from it.
	// Sensor object is created from the HMD, to ensure that it is on the
	// correct device.

#if defined(OVR_OS_WIN32)
	OVR::Thread::SetCurrentPriority(Thread::HighestPriority);

	if (OVR::Thread::GetCPUCount() >= 4) // Don't do this unless there are at least 4 processors, otherwise the process could hog the machine.
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	}
#endif

	ovr_Initialize();

	Hmd = ovrHmd_Create(0);

	if (!Hmd)
	{

		// If we didn't detect an Hmd, create a simulated one for debugging.
		Hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
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



	return 0;
}