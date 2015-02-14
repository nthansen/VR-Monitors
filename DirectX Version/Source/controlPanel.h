#include <windows.h>
#include <windowsx.h>

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	void createControlPanel(HINSTANCE hinst);

	bool getCloseApp();


private:

	HWND window;

	bool closeApp;

	void setupControlPanel();


};

extern ControlPanel controlPanel;
