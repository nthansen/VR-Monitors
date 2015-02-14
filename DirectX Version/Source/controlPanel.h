#include <windows.h>
#include <windowsx.h>

class ControlPanel{
	
public:

	ControlPanel();
	
	~ControlPanel();

	void createControlPanel(HINSTANCE hinst);

	void setupControlPanel();

	HWND window;

};