#include "controlPanel.h"

ControlPanel controlPanel;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND handle = nullptr;
	float dwPos = NULL;

	switch (msg)
	{
	case WM_HSCROLL:
		handle = (HWND)lParam;
			dwPos = SendMessage(handle, TBM_GETPOS, 0, 0);
			controlPanel.moveCameraZ((float)dwPos, handle);
		break;
	case WM_COMMAND:
		// if the quit button is clicked then destroy this window
		if (LOWORD(wParam) == 1) {
			controlPanel.~ControlPanel();
		}
		// if the user changes the background in the drop down menu
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			handle = (HWND)lParam;
			// If the user makes a selection from the list:
			//   Send CB_GETCURSEL message to get the index of the selected list item.
			int ItemIndex = SendMessage(handle, (UINT)CB_GETCURSEL,
				(WPARAM)0, (LPARAM)0);
			controlPanel.changeBackground(ItemIndex, handle);
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
		// must do to color all the static text to the background color
	case WM_CTLCOLORSTATIC:
		HDC hdcStatic = (HDC)wParam;

		SetTextColor(hdcStatic, RGB(0, 0, 0));
		SetBkMode(hdcStatic, COLOR_BACKGROUND);

		return (LRESULT)GetStockObject(GRAY_BRUSH);
	}
	return 0;
}
ControlPanel::ControlPanel() {
	window = nullptr;
	currScene = NULL;
	closeApp = false;
}

void ControlPanel::createControlPanel(HINSTANCE hinst, Scene * roomScene, Vector3f * pos) {

	currScene = roomScene;
	cameraPos = pos;

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
		// for the exit
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

		// create the text for the combox box
		CreateWindow(L"STATIC", L"Background:",
			SS_LEFT | WS_VISIBLE | WS_CHILD,
			11, 5,
			85, 20,
			window,
			NULL,
			(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
			NULL);


		// create the combo box for all the different options
		backgroundCombobox = CreateWindow(L"COMBOBOX", NULL,
			CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			10, 25, 100, 120, window, (HMENU)2, (HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
			NULL);

		// all the different options for the combo box
		TCHAR Backgrounds[5][16] =
		{
			TEXT("Evening"), TEXT("Sunny"), TEXT("Pier"), TEXT("Beach"), TEXT("Beach2"),
		};


		TCHAR A[16];
		memset(&A, 0, sizeof(A));
		for (int k = 0; k <= 4; k += 1)
		{
			wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)Backgrounds[k]);

			// Add string to combobox.
			SendMessage(backgroundCombobox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
		}

		// Send the CB_SETCURSEL message to display an initial item 
		//  in the selection field  
		SendMessage(backgroundCombobox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);


		FLOAT iMin = -4;     // minimum value in trackbar range 
		FLOAT iMax = 4;     // maximum value in trackbar range 
		FLOAT iSelMin = -4;  // minimum value in trackbar selection 
		FLOAT iSelMax = 4;  // maximum value in trackbar selection 

		cameraPositionTrackbar = CreateWindowEx(
			0,                               // no extended styles 
			TRACKBAR_CLASS,                  // class name 
			L"Trackbar Control",              // title (caption) 
			WS_CHILD |
			WS_VISIBLE |
			TBS_AUTOTICKS |
			TBS_ENABLESELRANGE,              // style 
			5, 60,                          // position 
			200, 30,                         // size 
			window,                         // parent window 
			(HMENU)3,                     // control identifier 
			(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),                         // instance 
			NULL                             // no WM_CREATE parameter 
			);

		SendMessage(cameraPositionTrackbar, TBM_SETRANGE,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)MAKELONG(iMin, iMax));  // min. & max. positions

		SendMessage(cameraPositionTrackbar, TBM_SETPAGESIZE,
			0, (LPARAM)4);                  // new page size 

		SendMessage(cameraPositionTrackbar, TBM_SETSEL,
			(WPARAM)FALSE,                  // redraw flag 
			(LPARAM)MAKELONG(iSelMin, iSelMax));

		SendMessage(cameraPositionTrackbar, TBM_SETPOS,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)0);

	}
}

void ControlPanel::moveCameraZ(float zValue, HWND identifier) {
	if (cameraPositionTrackbar == identifier){
		cameraPos->z = zValue * .2;
		currScene->Models[1]->Pos.z = zValue *.2;
	}
}

void ControlPanel::changeBackground(int background, HWND identifier){
	if (backgroundCombobox == identifier) {
		if (background == 0) {
			currScene->Models[1]->Fill = currScene->generated_texture[4];
		}
		else if (background == 1){
			currScene->Models[1]->Fill = currScene->generated_texture[5];
		}
		else if (background == 2) {
			currScene->Models[1]->Fill = currScene->generated_texture[6];
		}
		else if (background == 3){
			currScene->Models[1]->Fill = currScene->generated_texture[7];
		}
		else if (background == 4) {
			currScene->Models[1]->Fill = currScene->generated_texture[8];
		}
	}
}
