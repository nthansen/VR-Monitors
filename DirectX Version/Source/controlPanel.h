#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "OVR_CAPI.h"					// Include the OculusVR SDK
#include "scene.h"

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	// creates the actual window and recieves the scene and pos to use for later
	void createControlPanel(HINSTANCE hinst, Scene *roomScene, Vector3f *pos, ovrHmd * HMD, float * yaw);

	// changes the background based on the int given
	void changeBackground(int background);

	// moves the camera around 
	void moveCameraZ(float zValue);

	// find out if we need to close the app or not, used when user clicks the quit button
	bool getCloseApp();

	// check if the message given about a combobox is for the background
	// needed if we add more comboboxes
	bool checkIfBackgroundCombobox(HWND check);

	// check if the message given about a trackbar is for the camera position
	// needed if we add more trackbars
	bool checkIfCameraPositionTrackbar(HWND check);

	// Check to make sure we aren't currently moving the monitor
	bool movingMonitor;

	// used whenever we move the monitor since it needs to be constantly updating
	void updateControlPanel();

	void recenterOculus();

private:

	// holds the handle to each part of the control panel window
	HWND window;
	HWND backgroundCombobox;
	HWND cameraPositionTrackbar;
	ovrHmd * oculus;
	float * yaw;
	const float PI = 3.1415972f;

	// used so we can manipulate the stuff inside the scene and the camera
	Scene *currScene;
	Vector3f *cameraPos;

	// Value used to know if we need to close the application or not
	bool closeApp;

	// the function to actually move the monitor
	void moveMonitor();

	// sets up the control panel by calling the helper functions below
	void setupControlPanel();

	void createText();
	void createButtons();
	void createDropDowns();
	void createSliders();

};

// used to we can reference this instance of the control panel outside of it.
extern ControlPanel controlPanel;
