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

	void resizeMonitor(float resizeValue);

	// find out if we need to close the app or not, used when user clicks the quit button
	bool getCloseApp();

	// check if the message given about a combobox is for the background
	// needed if we add more comboboxes
	bool checkIfBackgroundCombobox(HWND check);

	// check if the message given about a trackbar is for the camera position
	// needed if we add more trackbars
	bool checkIfCameraPositionTrackbar(HWND check);

	bool checkIfResizeMonitorTrackbar(HWND check);

	// Check to make sure we aren't currently moving the monitor
	bool movingMonitor;

	// used whenever we move the monitor since it needs to be constantly updating
	void updateControlPanel();

	// adds a monitor to the scene
	bool addMonitor();

	void recenterOculus();

	// used to restore monitors in their original positions
	void resetMonitors();

	//used to initiate a monitor pick calls private function of the currScene pickMonitor
	//to set the pickedMonitor of the controlPanel
	void initPick();
private:

	// holds the handle to each part of the control panel window
	HWND window;
	HWND backgroundCombobox;
	HWND cameraPositionTrackbar;
	HWND monitorSizeTrackbar;
	ovrHmd * oculus;
	float * yaw;
	const float PI = 3.1415972f;
	Matrix4f *view;//for view matrix
	Matrix4f *proj;//for projection matrix

	// used so we can manipulate the stuff inside the scene and the camera
	Scene *currScene;
	Vector3f *cameraPos;

	//used to set the monitor to move
	int pickedMonitor = 0;

	// Value used to know if we need to close the application or not
	bool closeApp;

	// the function to actually move the monitor
	void moveMonitor(const int);

	// sets up the control panel by calling the helper functions below
	void setupControlPanel();

	void createText();
	void createButtons();
	void createDropDowns();
	void createSliders();

};

// used to we can reference this instance of the control panel outside of it.
extern ControlPanel controlPanel;
