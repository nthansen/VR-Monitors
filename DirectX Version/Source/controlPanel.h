#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "OVR_CAPI.h"					// Include the OculusVR SDK
#include "scene.h"
#include <strsafe.h>

#define WM_SYSICON          (WM_USER + 1)
#define ICO1 101
#define ID_TRAY_APP_ICON    1001
#define ID_TRAY_EXIT        1002
#define ID_TRAY_DESKTOP1	1003
#define ID_TRAY_DESKTOP2	1004
#define ID_TRAY_DESKTOP3	1005
#define ID_TRAY_DESKTOP4	1006


class ControlPanel{
	
public:
	void rotate(const float desktopNum);//rotates to the desktop that called it
	ControlPanel();
	
	~ControlPanel();

	// creates the actual window and recieves the scene and pos to use for later
	void createControlPanel(HINSTANCE hinst, Scene *roomScene, Vector3f *pos, ovrHmd * HMD, float * yaw, Matrix4f *view, Matrix4f *proj);

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
	bool rotatingMonitor;
	bool positioning;
	int  activeMonitor = 0;

	// used whenever we move the monitor since it needs to be constantly updating
	void updateControlPanel();

	// adds a monitor to the scene
	void addMonitor();

	void recenterOculus();

	void setUpSysTray();

	// used to restore monitors in their original positions
	void resetMonitors();

	HWND getWindow();

	NOTIFYICONDATA getSysTrayData();

	HMENU getSysTrayMenu();

	ControlPanel createNewControlPanel();

	void switchDesktop(int desktop);

private:

	// holds the handle to each part of the control panel window
	HWND window;
	HWND backgroundCombobox;
	HWND cameraPositionTrackbar;
	HWND monitorSizeTrackbar;
	HWND desktopRadio0;
	HWND desktopRadio1;
	HWND desktopRadio2;
	HWND desktopRadio3;
	HMENU sysTrayMenu;
	ovrHmd * oculus;
	float * yaw;
	Matrix4f * view;
	Matrix4f * proj;
	const float PI = 3.1415972f;

	int desktop;

	// used so we can manipulate the stuff inside the scene and the camera
	Scene *currScene;
	Vector3f *cameraPos;

	NOTIFYICONDATA cPI;

	// Value used to know if we need to close the application or not
	bool closeApp;

	// the function to actually move the monitor
	void moveMonitor();
	//rotates the monitor cube
	//void rotateMonitor(int);

	// sets up the control panel by calling the helper functions below
	void setupControlPanel();

	void createText();
	void createButtons();
	void createDropDowns();
	void createSliders();

	void resetDesktopRadio();

};

// used to we can reference this instance of the control panel outside of it.
extern ControlPanel controlPanel;
