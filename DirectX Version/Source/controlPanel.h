#include <windows.h>
#include <windowsx.h>
#include "scene.h"

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	void createControlPanel(HINSTANCE hinst, Scene &roomScene);

	void changeBackground(int itemNumber);

	bool getCloseApp();


private:

	HWND window;

	Scene * currScene;

	bool closeApp;

	void setupControlPanel();



};

extern ControlPanel controlPanel;
