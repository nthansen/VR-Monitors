#include "controlPanel.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

ControlPanel::ControlPanel() {
	window = nullptr;
}

ControlPanel::~ControlPanel() {
	if (window != nullptr){
		DestroyWindow(window);
	}
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
		// this message is read when the window is closed
	case WM_DESTROY:
	{
					   // close the application entirely
					   PostQuitMessage(0);
					   return 0;
	} break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}