#include "controlPanel.h"

ControlPanel controlPanel;


// used to process all the system messages sent for everything in this window
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// used to compare what item is being called when wParam can't
	HWND handle = nullptr;
	// used to get the position of the trackbar
	float dwPos = NULL;

	switch (msg)
	{
	case WM_HSCROLL:
		handle = (HWND)lParam;
		// see if it's the camera postion trackbar, if so then get the trackbar value and move the camera
		if (controlPanel.checkIfCameraPositionTrackbar(handle)) {
			dwPos = SendMessage(handle, TBM_GETPOS, 0, 0);
			controlPanel.moveCameraZ(dwPos);
		}
		break;
	case WM_COMMAND:
		// if the quit button is clicked then destroy this window
		if (LOWORD(wParam) == 1) {
			controlPanel.~ControlPanel();
		}
		// if the user changes the background in the drop down menu
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			handle = (HWND)lParam;
			if (controlPanel.checkIfBackgroundCombobox(handle)) {
				// If the user makes a selection from the list:
				//   Send CB_GETCURSEL message to get the index of the selected list item.
				int ItemIndex = SendMessage(handle, (UINT)CB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				controlPanel.changeBackground(ItemIndex);
			}
		}
		if (LOWORD(wParam) == 4) {
			controlPanel.recenterOculus();
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

// default constructor
ControlPanel::ControlPanel() {
	
	// set everything to null
	window = nullptr;
	backgroundCombobox = nullptr;
	cameraPositionTrackbar = nullptr;
	cameraPos = nullptr;
	currScene = NULL;
	oculus = nullptr;
	closeApp = false;
}

// actually creates the control panel window
// is given all the extra information necessary to process everything as well

void ControlPanel::createControlPanel(HINSTANCE hinst, Scene * roomScene, Vector3f * pos, ovrHmd * theOculus) {

	// now we have access to the information we need
	currScene = roomScene;
	cameraPos = pos;
	oculus = theOculus;

	// used to have a class for the window with the styles we want
	WNDCLASSW wc; 
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"Control Panel";
	wc.style = CS_OWNDC;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.cbWndExtra = NULL;
	RegisterClassW(&wc);

	// create the window and store it in our window object in the class
	window = CreateWindowW(L"Control Panel", L"VR-Monitors Control Panel", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		1000, 400, 500, 200, NULL, NULL, hinst, NULL);

	// now setup everything necessary for this
	setupControlPanel();
}

ControlPanel::~ControlPanel() {
	// make sure the parent class exists before trying to destroy it
	if (window != nullptr){
		// only need to call on the parent since the function takes care of destroying the children
		DestroyWindow(window);
	}
	// now we can notify the main to close the app
	closeApp = true;
}

// used in the main loop to see if we need to close the app or not
bool ControlPanel::getCloseApp() {
	return closeApp;
}

// returns true if the hwnd is the background combobox
bool ControlPanel::checkIfBackgroundCombobox(HWND check) {
	return backgroundCombobox == check;
}

// returns true if the hwnd is the camera position trackbar
bool ControlPanel::checkIfCameraPositionTrackbar(HWND check) {
	return cameraPositionTrackbar == check;
}

// calls all the helper functions for creating each part of the window
void ControlPanel::setupControlPanel() {
	// makes sure the window exists before trying to create stuff for it
	if (window != nullptr) {
		createText();
		createButtons();
		createSliders();
		createDropDowns();
	}
}

// creates all the buttons for the user to click on
void ControlPanel::createButtons() {
	// create the quit button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Quit VR-Monitors",      // Button text 
		WS_VISIBLE | WS_CHILD,  // Styles 
		350,         // x position 
		130,         // y position 
		125,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)1,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Recenter Oculus",      // Button text 
		WS_VISIBLE | WS_CHILD,  // Styles 
		10,         // x position 
		130,         // y position 
		125,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)4,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.
}

void ControlPanel::createDropDowns(){

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
}

void ControlPanel::createSliders() {
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
		5, 75,                          // position 
		200, 30,                         // size 
		window,                         // parent window 
		(HMENU)3,                     // control identifier 
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),      // instance 
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

void ControlPanel::createText() {
	// create the text for the background drop down
	CreateWindow(L"STATIC", L"Background:",
		SS_LEFT | WS_VISIBLE | WS_CHILD,
		11, 5,
		85, 20,
		window,
		NULL,
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);

	// create text for camera slider
	CreateWindow(L"STATIC", L"Move Camera:",
		SS_LEFT | WS_VISIBLE | WS_CHILD,
		11, 55,
		120, 20,
		window,
		NULL,
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);
}

// move the camera on the z coordinates
void ControlPanel::moveCameraZ(float zValue) {
		cameraPos->z = zValue * .2;
		currScene->Models[1]->Pos.z = zValue *.2;
}

// recenters the oculus
void ControlPanel::recenterOculus() {
	ovrHmd_RecenterPose(*oculus);
}

// change the background based on the value given
void ControlPanel::changeBackground(int background){
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
