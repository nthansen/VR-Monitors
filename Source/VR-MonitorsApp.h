#ifndef VRMonitors
#define VRMonitors

#include "OVR_Kernel.h"

#include "../Assets/CommonSrc/Platform/Platform_Default.h"
#include "../Assets/CommonSrc/Render/Render_Device.h"
#include "../Assets/CommonSrc/Render/Render_XmlSceneLoader.h"
#include "../Assets/CommonSrc/Platform/Gamepad.h"
#include "../Assets/CommonSrc/Util/OptionMenu.h"
#include "../Assets/CommonSrc/Util/RenderProfiler.h"

#include "Util/Util_Render_Stereo.h"
using namespace OVR::Util::Render;

#include "Kernel/OVR_DebugHelp.h"


#include "Sensors/OVR_DeviceConstants.h"

// Filename to be loaded by default, searching specified paths.
#define WORLDDEMO_ASSET_FILE  "VR-Monitors.xml"

#define WORLDDEMO_ASSET_PATH1 "Assets/VR-Monitors/"
#define WORLDDEMO_ASSET_PATH2 "../../Assets/VR-Monitors/"

using namespace OVR;
using namespace OVR::OvrPlatform;
using namespace OVR::Render;

class VRMonitorsApp : public Application
{
public:

	VRMonitorsApp();
	~VRMonitorsApp();

private: 

	// These fivev functions are virtual because we are overriding the functions inside the platform.h/.cpp files

	virtual int  OnStartup(int argc, const char** argv);
	//virtual void OnIdle();

	//virtual void OnMouseMove(int x, int y, int modifiers);
	virtual void OnKey(OVR::KeyCode key, int chr, bool down, int modifiers);
	//virtual void OnResize(int width, int height);

	bool         SetupWindowAndRendering(int argc, const char** argv);

private:

	ovrHmd	Hmd;
	RenderDevice*       pRender;
	RendererParams      RenderParams;
	Sizei	WindowSize;

	enum SceneRenderMode
	{
		Scene_World
	};
	SceneRenderMode    SceneMode;


};


#endif