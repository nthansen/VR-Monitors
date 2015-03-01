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
		// Our user defined WM_SYSICON message.
	case WM_SYSICON:
	{

					   switch (wParam)
					   {
					   case ID_TRAY_APP_ICON:
						   SetForegroundWindow(controlPanel.getWindow());
						   break;
					   }

					   if (lParam == WM_LBUTTONUP)
					   {

						   ShowWindow(controlPanel.getWindow(), SW_RESTORE);
					   }
					   else if (lParam == WM_RBUTTONDOWN)
					   {
						   // Get current mouse position.
						   POINT curPoint;
						   GetCursorPos(&curPoint);
						   SetForegroundWindow(controlPanel.getWindow());

						   // TrackPopupMenu blocks the app until TrackPopupMenu returns

						   UINT clicked = TrackPopupMenu(controlPanel.getSysTrayMenu(), TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

						   SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
						   if (clicked == ID_TRAY_EXIT)
						   {
							   // quit the application.
							   controlPanel.~ControlPanel();
							   PostQuitMessage(0);
						   }
						   else if (clicked == ID_TRAY_DESKTOP1) {
							   /*currScene->Models[0]->Pos = *cameraPos;
							   currScene->Models[0]->Rot = Quatf(Vector3f(0, .001, 0), 3.1415927);*/
							   controlPanel.rotate(1.0);
						   }
						   else if (clicked == ID_TRAY_DESKTOP2) {
							   // switch to Desktop2
							   controlPanel.rotate(2.0);
							   controlPanel.switchDesktop(1);
						   }
						   else if (clicked == ID_TRAY_DESKTOP3) {
							   // switch to Desktop3
							   controlPanel.rotate(3.0);
						   }
						   else if (clicked == ID_TRAY_DESKTOP4) {
							   // switch to Desktop4
							   controlPanel.rotate(4.0);
						   }
					   }
					   break;
	}

	case WM_HSCROLL:
		handle = (HWND)lParam;
		// see if it's the camera postion trackbar, if so then get the trackbar value and move the camera
		if (controlPanel.checkIfCameraPositionTrackbar(handle)) {
			dwPos = SendMessage(handle, TBM_GETPOS, 0, 0);
			controlPanel.moveCameraZ(dwPos);
		}
		/*
		else if (controlPanel.checkIfResizeMonitorTrackbar(handle)) {
			dwPos = SendMessage(handle, TBM_GETPOS, 0, 0);
			controlPanel.resizeMonitor(dwPos);
		}*/
		break;
	case WM_COMMAND:
		// if the quit button is clicked then destroy this window
		if (LOWORD(wParam) == 1) {
			controlPanel.~ControlPanel();
		}
		// if the user changes the background in the drop down menu
		else if (HIWORD(wParam) == CBN_SELCHANGE) {
			handle = (HWND)lParam;
			if (controlPanel.checkIfBackgroundCombobox(handle)) {
				// If the user makes a selection from the list:
				//   Send CB_GETCURSEL message to get the index of the selected list item.
				int ItemIndex = SendMessage(handle, (UINT)CB_GETCURSEL,
					(WPARAM)0, (LPARAM)0);
				controlPanel.changeBackground(ItemIndex);
			}
		}
		// user clicked the recenter oculus button
		else if (LOWORD(wParam) == 4) {
			controlPanel.recenterOculus();
		}
		// user clicked the move monitor button
		else if (LOWORD(wParam) == 5) {
			handle = (HWND)lParam;
			if (controlPanel.movingMonitor) {
				controlPanel.movingMonitor = false;
				Button_SetText(handle, L"Move Monitor");
			}
			else {
				controlPanel.movingMonitor = true;
				Button_SetText(handle, L"Place Monitor");
			}
		}
		// user clicked the reset monitor button
		else if (LOWORD(wParam) == 6) {
			controlPanel.resetMonitors();
		}
		// user clicked the add monitor button
		else if (LOWORD(wParam) == 8) {
			controlPanel.addMonitor();
		}
		break;
	case WM_CLOSE:
		controlPanel.~ControlPanel();
		break;
	case WM_DESTROY:
		controlPanel.~ControlPanel();
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

//rotate 
//called from system menu, rotates to the selected monitor
void ControlPanel::rotate(float monitorNum){
	// rotate the cube by pi/2 times the monitor side we want
	//float transition = .9;
	//int control = 300;
	//Quatf temp = currScene->Models[0]->Rot;
	//for (transition; transition < control; transition+=transition){
		//currScene->Models[0]->Rot = Quatf(Vector3f(0, .00001, 0), 3.14159 / 2 * monitorNum);
	//}
		currScene->Models[0]->Rot = Quatf(Vector3f(0, .000001, 0), PI / 2 * monitorNum);
		Model *mod = currScene->Models[0];
		Matrix4f  modmat = mod->GetMatrix();
		mod->Pos = modmat.Transform(Vector3f(-2, 0, 0));
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
	movingMonitor = false;
}

ControlPanel ControlPanel::createNewControlPanel() {
	ControlPanel anotherControlPanel;
	anotherControlPanel.createControlPanel(NULL, currScene, cameraPos, oculus, yaw);
	return anotherControlPanel;
}

// actually creates the control panel window
// is given all the extra information necessary to process everything as well

void ControlPanel::createControlPanel(HINSTANCE hinst, Scene * roomScene, Vector3f * pos, ovrHmd * theOculus, float * yaw) {

	// now we have access to the information we need
	currScene = roomScene;
	cameraPos = pos;
	oculus = theOculus;
	this->yaw = yaw;

	// used to have a class for the window with the styles we want
	WNDCLASSW wc; 
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"Control Panel";
	wc.style = CS_OWNDC;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	wc.cbWndExtra = NULL;
	RegisterClassW(&wc);

	// create the window and store it in our window object in the class
	window = CreateWindowW(L"Control Panel", L"VR-Monitors Control Panel", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		1000, 400, 500, 200, NULL, NULL, hinst, NULL);

	// now setup everything necessary for this
	setupControlPanel();

	setUpSysTray();
}

void ControlPanel::setUpSysTray() {

	sysTrayMenu = CreatePopupMenu();
	
	AppendMenu(sysTrayMenu, MF_STRING, ID_TRAY_DESKTOP1, TEXT("Desktop 1"));

	AppendMenu(sysTrayMenu, MF_STRING, ID_TRAY_DESKTOP2, TEXT("Desktop 2"));

	AppendMenu(sysTrayMenu, MF_STRING, ID_TRAY_DESKTOP3, TEXT("Desktop 3"));

	AppendMenu(sysTrayMenu, MF_STRING, ID_TRAY_DESKTOP4, TEXT("Desktop 4"));

	AppendMenu(sysTrayMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit VR-Monitors"));

	cPI = {};

	cPI.cbSize = sizeof(cPI);

	cPI.hWnd = window;

	cPI.uID = ID_TRAY_APP_ICON;

	cPI.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	cPI.uCallbackMessage = WM_SYSICON;

	HINSTANCE hDll = LoadLibrary(L"SHELL32.dll");

	cPI.hIcon = (HICON)LoadIcon(hDll, MAKEINTRESOURCE(3));

	StringCchCopy(cPI.szTip, ARRAYSIZE(cPI.szTip), L"VR-Monitors Control Panel");

	cPI.dwInfoFlags = NIIF_INFO;

	bool success = Shell_NotifyIcon(NIM_ADD, &cPI);
}

ControlPanel::~ControlPanel() {
	// make sure the parent class exists before trying to destroy it
	if (window != nullptr){
		// only need to call on the parent since the function takes care of destroying the children
		DestroyWindow(window);
	}
	Shell_NotifyIcon(NIM_DELETE, &controlPanel.getSysTrayData());
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

// returns true if the hwnd is the resize monitor trackbar
bool ControlPanel::checkIfResizeMonitorTrackbar(HWND check) {
	return monitorSizeTrackbar == check;
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

	// create the recenter oculus button
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

	// create the move monitor button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Move Monitor",      // Button text 
		WS_VISIBLE | WS_CHILD,  // Styles 
		375,         // x position 
		26,         // y position 
		100,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)5,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the reset monitors
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Reset Monitors",      // Button text 
		WS_VISIBLE | WS_CHILD,  // Styles 
		240,         // x position 
		26,         // y position 
		110,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)6,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding monitor button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Add Monitor",      // Button text 
		WS_VISIBLE | WS_CHILD,  // Styles 
		375,         // x position 
		75,         // y position 
		100,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)8,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// used to set the desktop 1 to true at the start
	HWND desktop1;
	// create the adding desktop 1 button
	desktop1 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"1",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		235,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)9,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	SendMessage(desktop1, BM_SETCHECK, BST_CHECKED, 0);

	// create the adding desktop 2 button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"2",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		268,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)10,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 3 button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"3",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		300,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)11,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 4 button
	CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"4",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		335,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)12,       // used for the wndProc to know what button is pressed
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
	FLOAT iMin = -5;     // minimum value in trackbar range 
	FLOAT iMax = -1;     // maximum value in trackbar range 
	FLOAT iSelMin = -5;  // minimum value in trackbar selection 
	FLOAT iSelMax = -1;  // maximum value in trackbar selection 

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
		0, (LPARAM)1);                  // new page size 

	SendMessage(cameraPositionTrackbar, TBM_SETSEL,
		(WPARAM)FALSE,                  // redraw flag 
		(LPARAM)MAKELONG(iSelMin, iSelMax));

	SendMessage(cameraPositionTrackbar, TBM_SETPOS,
		(WPARAM)TRUE,                   // redraw flag 
		(LPARAM)-3);


	/*
	iMin = -1;     // minimum value in trackbar range 
	iMax = 3;     // maximum value in trackbar range 
	iSelMin = -1;  // minimum value in trackbar selection 
	iSelMax = 3;  // maximum value in trackbar selection 

	monitorSizeTrackbar = CreateWindowEx(
		0,                               // no extended styles 
		TRACKBAR_CLASS,                  // class name 
		L"Trackbar Control",              // title (caption) 
		WS_CHILD |
		WS_VISIBLE |
		TBS_AUTOTICKS |
		TBS_ENABLESELRANGE,              // style 
		230, 75,                          // position 
		130, 30,                         // size 
		window,                         // parent window 
		(HMENU)7,                     // control identifier 
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),      // instance 
		NULL                             // no WM_CREATE parameter 
		);

	SendMessage(monitorSizeTrackbar, TBM_SETRANGE,
		(WPARAM)TRUE,                   // redraw flag 
		(LPARAM)MAKELONG(iMin, iMax));  // min. & max. positions

	SendMessage(monitorSizeTrackbar, TBM_SETPAGESIZE,
		0, (LPARAM)4);                  // new page size 

	SendMessage(monitorSizeTrackbar, TBM_SETSEL,
		(WPARAM)FALSE,                  // redraw flag 
		(LPARAM)MAKELONG(iSelMin, iSelMax));

	SendMessage(monitorSizeTrackbar, TBM_SETPOS,
		(WPARAM)TRUE,                   // redraw flag 
		(LPARAM)1);
		*/
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

	CreateWindow(L"STATIC", L"Choose Desktop:",
		SS_LEFT | WS_VISIBLE | WS_CHILD,
		239, 60,
		120, 20,
		window,
		NULL,
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);

	/*
	// create text for camera slider
	CreateWindow(L"STATIC", L"Resize Monitor:",
		SS_LEFT | WS_VISIBLE | WS_CHILD,
		241, 55,
		120, 20,
		window,
		NULL,
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);
		*/
}

// move the camera on the z coordinates
void ControlPanel::moveCameraZ(float zValue) {
		cameraPos->z = zValue;
		currScene->Models[1]->Pos.z = zValue;
}

void ControlPanel::resizeMonitor(float resizeValue){
	currScene->Models[0]->scale = resizeValue;
	//resizeValue *= .8;
	//Vector3f pos = currScene->Models[0]->Pos;
	//currScene->Models[0]->AddSolidColorBox(pos.x - resizeValue, pos.y - resizeValue, pos.z, pos.x + resizeValue, pos.y + resizeValue, pos.z, Model::Color(128, 128, 128));
	//currScene->Models[0]->AllocateBuffers();
}

void ControlPanel::updateControlPanel() {
	if (movingMonitor) {
		moveMonitor();
	}
}

//rotate the object about the y-axis (or very close) based on the depth of the object at the angle described
//since the object spawns in front of us on the z axis and we are now facing the direction of positive x axis
//we must offset this to rotate negative pi radians so the object will be in front of us
void ControlPanel::moveMonitor() {
	currScene->Models[0]->Pos = *cameraPos;
	currScene->Models[0]->Rot = Quatf(Vector3f(0, .001, 0), -PI + *yaw);
}

void ControlPanel::resetMonitors() {
	// eventualyl will be a for loop
	currScene->Models[0]->setOriginalPos();
}

void ControlPanel::addMonitor() {
	currScene->addMonitor(*yaw, *cameraPos);
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

HWND ControlPanel::getWindow() {
	return window;
}

NOTIFYICONDATA ControlPanel::getSysTrayData() {
	return cPI;
}

HMENU ControlPanel::getSysTrayMenu() {
	return sysTrayMenu;
}

void ControlPanel::switchDesktop(int desktop) {
	currScene->doRender = false;
	currScene->Models[0]->desktop->newDesktop();
	currScene->doRender = true;
}