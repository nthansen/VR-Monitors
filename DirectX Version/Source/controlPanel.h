#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "scene.h"

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	void createControlPanel(HINSTANCE hinst, Scene *roomScene, Vector3f *pos);

	void changeBackground(int itemNumberm, HWND identifier);

	void moveCameraZ(float zValue, HWND identifier);

	bool getCloseApp();


private:

	HWND window;

	HWND backgroundCombobox;

	HWND cameraPositionTrackbar;

	Scene *currScene;

	Vector3f *cameraPos;

	bool closeApp;

	void setupControlPanel();



};

extern ControlPanel controlPanel;
