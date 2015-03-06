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
							   //intial attempt
							   /*currScene->Models[0]->Pos = *cameraPos;
							   currScene->Models[0]->Rot = Quatf(Vector3f(0, .001, 0), 3.1415927);*/

							   //second attempt works but doesnt do the transition smoothly
							   //controlPanel.rotate(1.0);

							   //third attempt set active monitor here and set rotating monitor to true
							   controlPanel.activeMonitor = 0;
							   controlPanel.rotatingMonitor = true; //sets bool to rotate active monitor next update
							   controlPanel.firstRotate = true;


						   }
						   else if (clicked == ID_TRAY_DESKTOP2) {
							   // switch to Desktop2
							   controlPanel.activeMonitor = 1;
							   controlPanel.rotatingMonitor = true; //sets bool to rotate active monitor next update
							   controlPanel.firstRotate = true;

						   }
						   else if (clicked == ID_TRAY_DESKTOP3) {
							   // switch to Desktop3
							   controlPanel.activeMonitor = 2;
							   controlPanel.rotatingMonitor = true; //sets bool to rotate active monitor next update
							   controlPanel.firstRotate = true;
						   }
						   else if (clicked == ID_TRAY_DESKTOP4) {
							   // switch to Desktop4
							   controlPanel.activeMonitor = 3;
								controlPanel.rotatingMonitor = true; //sets bool to rotate active monitor next update
								controlPanel.firstRotate = true;

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
		if (LOWORD(wParam) == ID_QUIT) {
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
		else if (LOWORD(wParam) == ID_RECENTER) {
			controlPanel.recenterOculus();
		}
		// user clicked the move monitor button
		else if (LOWORD(wParam) == ID_MOVE_MONITOR) {
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
		else if (LOWORD(wParam) == ID_RESET_MONITOR) {
			controlPanel.resetMonitors();
		}
		// user clicked the add monitor button
		else if (LOWORD(wParam) == ID_ADD_MONITOR) {
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
//passed the active monitor from update control panel and checks every update if rotating monitor
void ControlPanel::rotate(float monitorNum){

	if (desktop == monitorNum) {
		return;
	}

	Model *mod = currScene->Models[0];//pointer to the monitor cube
	// rotate the cube by pi/2 times the monitor side we want
	//need to rotate the cube a little bit each update until the rotation
	//is equal to the complete rotation of the active monitor
	
	mod->Rot.y;
	if (firstRotate){//&&currScene->Models[0]->Rot.Angle(Quatf(Vector3f(0, .00001, 0), 3.14159/2*3)) > 0.01){
		currScene->Models[0]->Rot = currScene->Models[0]->Rot.Nlerp(Quatf(Vector3f(0, .000001, 0), PI),.9);//do the rotation
		
		//check if we are done rotating and set the final rotated matrix
		if (currScene->Models[0]->Rot.Angle(Quatf(Vector3f(0, -1, 0), PI)) <= 0.01){
			currScene->Models[0]->rotatedMatrix = currScene->Models[0]->GetMatrix();
			//positionng = true;//now we need to position if we want any additional future transforms do it there
			firstRotate = false;

			controlPanel.switchDesktop(activeMonitor);

			secondRotate = true;
		}
		
		
	}
	//do the second rotation
	else if (secondRotate){//&&currScene->Models[0]->Rot.Angle(Quatf(Vector3f(0, .00001, 0), 3.14159*2)) > 0.01){
		currScene->Models[0]->Rot = Quatf(Vector3f(0, .000001, 0), PI*2);//do the rotation
		//check if we are finished with the second rotation
		//if (currScene->Models[0]->Rot.Angle(Quatf(Vector3f(0, -1, 0), PI)) <= 0.01){
			secondRotate = false;
		//}

	}
	//positioning keeps us from moving a monitor that is already chosen or equal to the rotation above
	//we can keep this feature if we would like to move the cube around while it is rotating
	else if(positioning){
			positioning = false;//we are done moving set to false
	}
	//if the angles are the same then we are done rotating so set the bool to false so
	//that update monitor wont call this function until the next rotate monitor call
	else{
		//if we want to change the position of the monitor we can do so here just before we are finished
		//we can also jump to a totally different monitor at the last minute
		
		rotatingMonitor = false;//we are finished changing the monitor position so dont call from updatecontrol panel anymore
	}
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
	anotherControlPanel.createControlPanel(NULL, currScene, cameraPos, oculus, yaw, view, proj);
	return anotherControlPanel;
}

// actually creates the control panel window
// is given all the extra information necessary to process everything as well

void ControlPanel::createControlPanel(HINSTANCE hinst, Scene * roomScene, Vector3f * pos, ovrHmd * theOculus, float * yaw, Matrix4f * _view, Matrix4f * _proj) {

	// now we have access to the information we need
	currScene = roomScene;
	cameraPos = pos;
	oculus = theOculus;
	this->yaw = yaw;
	this->view = _view;
	this->proj = _proj;
	desktop = currScene->Models[0]->desktop->outputNumber;

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

	if (desktop == 0) {
		CheckMenuItem(sysTrayMenu, ID_TRAY_DESKTOP1, MF_CHECKED);
	}
	else if (desktop == 1) {
		CheckMenuItem(sysTrayMenu, ID_TRAY_DESKTOP2, MF_CHECKED);
	} else if (desktop == 2) {
		CheckMenuItem(sysTrayMenu, ID_TRAY_DESKTOP3, MF_CHECKED);
	}
	else if (desktop == 3) {
		CheckMenuItem(sysTrayMenu, ID_TRAY_DESKTOP4, MF_CHECKED);
	}

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
		(HMENU)ID_QUIT,       // used for the wndProc to know what button is pressed
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
		(HMENU)ID_RECENTER,       // used for the wndProc to know what button is pressed
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
		(HMENU)ID_MOVE_MONITOR,       // used for the wndProc to know what button is pressed
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
		(HMENU)ID_RESET_MONITOR,       // used for the wndProc to know what button is pressed
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
		(HMENU)ID_ADD_MONITOR,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 1 button
	desktopRadio0 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"1",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		235,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)ID_DESKTOP1_RADIO,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 2 button
	desktopRadio1 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"2",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		268,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)ID_DESKTOP2_RADIO,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 3 button
	desktopRadio2 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"3",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		300,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)ID_DESKTOP3_RADIO,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	// create the adding desktop 4 button
	desktopRadio3 = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"4",      // Button text 
		BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD,  // Styles 
		335,         // x position 
		75,         // y position 
		30,        // Button width
		25,        // Button height
		window,     // Parent window
		(HMENU)ID_DESKTOP4_RADIO,       // used for the wndProc to know what button is pressed
		(HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	resetDesktopRadio();
}

void ControlPanel::createDropDowns(){

	// create the combo box for all the different options
	backgroundCombobox = CreateWindow(L"COMBOBOX", NULL,
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		10, 25, 100, 120, window, (HMENU)ID_BACKGROUND, (HINSTANCE)GetWindowLong(window, GWL_HINSTANCE),
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
	FLOAT iMin = -2;     // minimum value in trackbar range 
	FLOAT iMax = 2;     // maximum value in trackbar range 
	FLOAT iSelMin = -2;  // minimum value in trackbar selection 
	FLOAT iSelMax = 2;  // maximum value in trackbar selection 

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
		(HMENU)ID_CAMERA_POS,                     // control identifier 
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
		(LPARAM)0);


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
	int defaultDistance = -1;
	cameraPos->z = zValue *.1 + defaultDistance;
	currScene->Models[1]->Pos.z = zValue * .1 + defaultDistance;
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
	if (rotatingMonitor){
		rotate(activeMonitor);
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
	currScene->Models[0]->desktop->newDesktop(desktop);
	currScene->doRender = true;
}

void ControlPanel::resetDesktopRadio() {
	if (desktop == 0) {
		SendMessage(desktopRadio0, BM_SETCHECK, BST_CHECKED, 0);
	}
	else if (desktop == 1) {
		SendMessage(desktopRadio1, BM_SETCHECK, BST_CHECKED, 0);
	}
	else if (desktop == 2) {
		SendMessage(desktopRadio2, BM_SETCHECK, BST_CHECKED, 0);
	}
	else if (desktop == 3) {
		SendMessage(desktopRadio3, BM_SETCHECK, BST_CHECKED, 0);
	}
}