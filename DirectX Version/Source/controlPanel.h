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
#define ID_QUIT				1007
#define ID_RECENTER			1008
#define ID_MOVE_MONITOR		1009
#define	ID_RESET_MONITOR	1010
#define	ID_ADD_MONITOR		1011
#define	ID_DESKTOP1_RADIO	1012
#define	ID_DESKTOP2_RADIO	1013
#define	ID_DESKTOP3_RADIO	1014
#define	ID_DESKTOP4_RADIO	1015
#define ID_BACKGROUND		1016
#define ID_CAMERA_POS		1017


class ControlPanel{
	
public:
	void rotate(const int desktopNum);//rotates to the desktop that called it
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
	bool firstRotate;//rotate the cube 180 degrees facing the back
	bool secondRotate;//roate the cube the last 180 degrees to face the original position
	bool positioning;
	int  activeMonitor = 0;
	int pickedMonitor = 0;

	// used whenever we move the monitor since it needs to be constantly updating
	void updateControlPanel();

	// adds a monitor to the scene
	void addMonitor();
	void moveMonitor(int);
	int initPick();//calls the pickmonitor method of this scene and sets the picked monitor, called by add monitor button
	void recenterOculus();

	void setUpSysTray();

	// used to restore monitors in their original positions
	void resetMonitors();

	HWND getWindow();

	NOTIFYICONDATA getSysTrayData();

	HMENU getSysTrayMenu();

	void switchDesktop(int desktop);

	void resetDesktopRadio();

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
	HWND addMonitorButton;
	HMENU sysTrayMenu;
	ovrHmd * oculus;
	float * yaw;
	Matrix4f * view;
	Matrix4f * proj;
	const float PI = 3.1415972f;

	int totalMonitors;
	int currentMonitors;

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

	void checkMonitors();

};

// used to we can reference this instance of the control panel outside of it.
extern ControlPanel controlPanel;
