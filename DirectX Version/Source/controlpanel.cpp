#include "controlPanel.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
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
}

void ControlPanel::createControlPanel(HINSTANCE hinst) {

	WNDCLASSW wc; 
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"Control Panel";
	wc.style = CS_OWNDC;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.cbWndExtra = NULL;
	RegisterClassW(&wc);

	window = CreateWindowW(L"Control Panel", L"VR-Monitors Control Panel", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		1000, 400, 500, 200, NULL, NULL, hinst, NULL);

	UpdateWindow(window);

}

ControlPanel::~ControlPanel() {
	if (window != nullptr){
		DestroyWindow(window);
	}
}

void ControlPanel::setupControlPanel() {

	if (window != nullptr) {
		HWND hwndButton = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"OK",      // Button text 
			WS_VISIBLE,  // Styles 
			600,         // x position 
			600,         // y position 
			100,        // Button width
			100,        // Button height
			window,     // Parent window
			NULL,       // No menu.
			(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
			NULL);      // Pointer not needed.
	}
}
