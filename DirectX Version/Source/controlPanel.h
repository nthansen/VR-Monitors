#include <windows.h>
#include <windowsx.h>
#include "scene.h"

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	void createControlPanel(HINSTANCE hinst, Scene *roomScene, Vector3f *pos);

	void changeBackground(int itemNumber);

	bool getCloseApp();


private:

	HWND window;

	Scene *currScene;

	Vector3f *cameraPos;

	bool closeApp;

	void setupControlPanel();



};

extern ControlPanel controlPanel;
