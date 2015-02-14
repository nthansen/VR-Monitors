#include "controlPanel.h"

ControlPanel controlPanel;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		// if the quit button is clicked then destroy this window
		if (LOWORD(wParam) == 1) {
			controlPanel.~ControlPanel();
		}
		break;
	case WM_CLOSE:
		controlPanel.~ControlPanel();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
ControlPanel::ControlPanel() {
	window = nullptr;
	closeApp = false;
}

void ControlPanel::createControlPanel(HINSTANCE hinst) {

	WNDCLASSW wc; 
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"Control Panel";
	wc.style = CS_OWNDC;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.cbWndExtra = NULL;
	RegisterClassW(&wc);

	window = CreateWindowW(L"Control Panel", L"VR-Monitors Control Panel", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		1000, 400, 500, 200, NULL, NULL, hinst, NULL);

	setupControlPanel();
}

ControlPanel::~ControlPanel() {
	if (window != nullptr){
		closeApp = true;
		DestroyWindow(window);
	}
}

bool ControlPanel::getCloseApp() {
	return closeApp;
}

void ControlPanel::setupControlPanel() {

	if (window != nullptr) {
		CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"Quit VR-Monitors",      // Button text 
			WS_VISIBLE | WS_CHILD,  // Styles 
			10,         // x position 
			130,         // y position 
			125,        // Button width
			25,        // Button height
			window,     // Parent window
			(HMENU)1,       // used for the wndProc to know what button is pressed
			(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.
		
	}
}
