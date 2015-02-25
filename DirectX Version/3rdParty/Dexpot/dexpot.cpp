/*
 *  Plugin Interface for Dexpot 1.5
 *  Copyright 2009-2010 Dexpot GbR
 *  
 *  Feel free to use this code in any way you like.
 */

#include "dexpot.h"

#define FILEMAP_SIZE 1024

std::map<HWND, Dexpot*> Dexpot::HandledPlugins;

Dexpot::Dexpot(LPCTSTR Name)
{
	size_t length = _tcslen(Name) + 1;
	PluginName = new TCHAR[length];
	_tcscpy_s(PluginName, length, Name);
	
	Connected = FALSE;
	hWndCallback = NULL;
	Filemap = NULL;
	Shared = NULL;
	FilemapMutex = NULL;
	hInstance = GetInstance();

	OnSwitch = NULL;
	OnSwitched = NULL;
	OnShutdown = NULL;
	OnLoad = NULL;
	OnHotkey = NULL;
	OnSetIntegerOption = NULL;
	OnSetStringOption = NULL;
	OnConfigure = NULL;
	OnSwitchRequest = NULL;
	OnDesktopCountChanged = NULL;
	OnMenuCommand = NULL;

	EventHandler = NULL;
}

Dexpot::~Dexpot()
{
	if(Connected) Disconnect();
	delete[] PluginName;
}

void Dexpot::RegisterEventHandler(DexpotEventHandler *Handler)
{
	EventHandler = Handler;
}

VOID Dexpot::SubscribeHooxpotEvents()
{
	if(EventHandler) SendNotifyMessage(hWndDexpot, DEX_SUBSCRIBEHOOXPOT, (WPARAM) hWndCallback, 0);
}

VOID Dexpot::UnsubscribeHooxpotEvents()
{
	SendNotifyMessage(hWndDexpot, DEX_UNSUBSCRIBEHOOXPOT, (WPARAM) hWndCallback, 0);
}

void Dexpot::RegisterOnSwitchRequestHandler(DWORD (*fp)(INT32 VonDesktop, INT32 NachDesktop, WORD Flags, WORD Trigger))
{
	OnSwitchRequest = fp;
}

void Dexpot::RegisterOnSwitchHandler(VOID (*fp)(INT32 VonDesktop, INT32 NachDesktop, WORD Flags, WORD Trigger))
{
	OnSwitch = fp;
}

void Dexpot::RegisterOnSwitchedHandler(VOID (*fp)(INT32 VonDesktop, INT32 NachDesktop, WORD Flags, WORD Trigger))
{
	OnSwitched = fp;
}

void Dexpot::RegisterOnShutdownHandler(VOID (*fp)(VOID))
{
	OnShutdown = fp;
}

void Dexpot::RegisterOnLoadHandler(VOID (*fp)(VOID))
{
	OnLoad = fp;
}

VOID Dexpot::RegisterOnHotkeyHandler(VOID (*fp)(INT32, WORD, WORD))
{
	OnHotkey = fp;
}

VOID Dexpot::RegisterOnSetIntegerOptionHandler(VOID (*fp)(INT32, INT32))
{
	OnSetIntegerOption = fp;
}

VOID Dexpot::RegisterOnSetStringOptionHandler(VOID (*fp)(INT32, LPTSTR))
{
	OnSetStringOption = fp;
}

VOID Dexpot::RegisterOnConfigureHandler(VOID (*fp)(VOID))
{
	OnConfigure = fp;
}

VOID Dexpot::RegisterOnDesktopCountChangedHandler(VOID (*fp)(INT32))
{
	OnDesktopCountChanged = fp;
}

VOID Dexpot::RegisterOnMenuCommandHandler(VOID (*fp)(INT32))
{
	OnMenuCommand = fp;
}

VOID Dexpot::RegisterHotkey(INT32 id, LPCTSTR name)
{
	COPYDATASTRUCT cds;
	DWORD len = sizeof(id) + _tcslen(name) * sizeof(TCHAR);
	BYTE *buffer = new BYTE[len];

	memcpy(buffer, &id, sizeof(id));
	memcpy((INT32*)buffer + 1, name, _tcslen(name) * sizeof(TCHAR));

	cds.dwData = DEX_REGISTERHOTKEY;
	cds.lpData = buffer;
	cds.cbData = len;

	SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);

	delete[] buffer;
}

VOID Dexpot::RegisterOption(BYTE type, INT32 id, LPCTSTR name)
{
	COPYDATASTRUCT cds;
	DWORD len = sizeof(type) + sizeof(id) + _tcslen(name) * sizeof(TCHAR);
	BYTE *buffer = new BYTE[len];
	VOID *p = buffer;

	memcpy(p, &type, sizeof(type));
	p = (BYTE*)p + 1;
	memcpy(p, &id, sizeof(id));
	p = (INT32*)p + 1;
	memcpy(p, name, _tcslen(name) * sizeof(TCHAR));

	cds.dwData = DEX_REGISTEROPTION;
	cds.lpData = buffer;
	cds.cbData = len;

	SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);

	delete[] buffer;
}

VOID Dexpot::SetOptionValue(BYTE type, INT32 id, LPVOID value)
{
	COPYDATASTRUCT cds;
	BYTE *buffer;
	DWORD len;
	LPVOID p;

	if(type == DEX_OPTION_TYPE_INTEGER)
	{
		len = sizeof(BYTE) + 2 * sizeof(INT32);
		buffer = new BYTE[len];
		p = buffer;
		memcpy(p, &type, sizeof(type));
		p = (BYTE*)p + 1;
		memcpy(p, &id, sizeof(id));
		p = (INT32*)p + 1;
		memcpy(p, value, sizeof(INT32));
	}
	else if(type == DEX_OPTION_TYPE_STRING)
	{
		LPTSTR s = (LPTSTR) value;
		len = sizeof(BYTE) + sizeof(INT32) + _tcslen(s) * sizeof(TCHAR);
		buffer = new BYTE[len];
		p = buffer;
		memcpy(p, &type, sizeof(type));
		p = (BYTE*)p + 1;
		memcpy(p, &id, sizeof(id));
		p = (INT32*)p + 1;
		memcpy(p, s, _tcslen(s) * sizeof(TCHAR));
	}

	cds.cbData = len;
	cds.lpData = buffer;
	cds.dwData = DEX_SETOPTIONVALUE;
	SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);

	delete[] buffer;
}

VOID Dexpot::SetOptionValue(INT32 id, INT32 value)
{
	SetOptionValue(DEX_OPTION_TYPE_INTEGER, id, &value);
}

VOID Dexpot::SetOptionValue(INT32 id, LPTSTR value)
{
	SetOptionValue(DEX_OPTION_TYPE_STRING, id, value);
}

VOID Dexpot::SetHotkeyValue(INT32 id, INT32 modifier, INT32 vkey)
{
	COPYDATASTRUCT cds;
	BYTE *buffer;
	DWORD len;
	LPVOID p;

	len = 3 * sizeof(INT32);
	buffer = new BYTE[len];
	p = buffer;
	memcpy(p, &id, sizeof(id));
	p = (INT32*)p + 1;
	memcpy(p, &modifier, sizeof(modifier));
	p = (INT32*)p + 1;
	memcpy(p, &vkey, sizeof(vkey));

	cds.cbData = len;
	cds.lpData = buffer;
	cds.dwData = DEX_SETHOTKEYVALUE;
	SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);

	delete[] buffer;
}

VOID Dexpot::InsertMainMenuItem(INT32 id, LPCTSTR label, HICON hIcon)
{
	COPYDATASTRUCT cds;
    BYTE *buffer;
    DWORD len = sizeof(id) + sizeof(hIcon) + _tcslen(label) * sizeof(TCHAR);
	LPVOID p;

	buffer = new BYTE[len];
	p = buffer;
    memcpy(p, &id, sizeof(id));
	p = (INT32*)p + 1;
    memcpy(p, &hIcon, sizeof(hIcon));
	p = (HICON*)p + 1;
	memcpy(p, label, _tcslen(label) * sizeof(TCHAR));

    cds.cbData = len;
    cds.dwData = DEX_INSERTMAINMENUITEM;
    cds.lpData = buffer;
    SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);

	delete[] buffer;
}

VOID Dexpot::RemoveMainMenuItem(INT32 id)
{
	SendNotifyMessage(hWndDexpot, DEX_REMOVEMAINMENUITEM, (WPARAM) hWndCallback, id);
}

VOID Dexpot::LoadSettings()
{
	SendMessage(hWndDexpot, DEX_LOADSETTINGS, PluginID, 0);
}

VOID Dexpot::SaveSettings()
{
	SendMessage(hWndDexpot, DEX_SAVESETTINGS, PluginID, 0);
}

HWND Dexpot::GetDexpotWindow()
{
	if(IsWindow(hWndDexpot)) return hWndDexpot;

	hWndDexpot = FindWindow(DEXPOTCLASS, DEXPOTTITLE);

    if(!IsWindow(hWndDexpot))
	{
		hWndDexpot = FindWindow(DEXPOTCLASS2, DEXPOTTITLE);
		if(!IsWindow(hWndDexpot)) return NULL;
	}

	return hWndDexpot;
}

BOOL Dexpot::DexpotRunning()
{
	return IsWindow(GetDexpotWindow());
}

DWORD Dexpot::GetPluginID()
{
	return PluginID;
}

HWND Dexpot::GetCallbackWindow()
{
	return hWndCallback;
}

BOOL Dexpot::Connect()
{
	COPYDATASTRUCT cds;
	TCHAR *szName;

	if(Connected && hWndDexpot == GetDexpotWindow()) return TRUE;
	if(!DexpotRunning()) return FALSE;

	hWndCallback = CreateCallbackWindow();

	cds.dwData = DEX_REGISTERPLUGIN;
	cds.lpData = PluginName;
	cds.cbData = _tcslen(PluginName) * sizeof(TCHAR);
    PluginID = SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
	Connected = PluginID > 0;

	if(Connected)
	{
		HandledPlugins.insert(std::pair<HWND, Dexpot*>(hWndCallback, this));
		size_t length = _tcslen(PluginName) + 19;
		szName = new TCHAR[length];
		_stprintf_s(szName, length, _T("Local\\DexpotPlugin%s"), PluginName);
		FilemapMutex = CreateMutex(NULL, FALSE, NULL);
		Filemap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILEMAP_SIZE, szName);
		if(Filemap) Shared = MapViewOfFile(Filemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(Shared)
		{
			cds.dwData = DEX_SETFILEMAP;
			cds.lpData = szName;
			cds.cbData = _tcslen(szName) * sizeof(TCHAR);
			SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
		}
		delete[] szName;
	}

	return Connected;
}

void Dexpot::Disconnect()
{
	if(Connected)
	{
		PostMessage(hWndDexpot, DEX_UNREGISTERPLUGIN, PluginID, 0);
        hWndDexpot = NULL;
		HandledPlugins.erase(hWndCallback);
		Connected = FALSE;
		DestroyWindow(hWndCallback);
		UnregisterClass(_T("Dexpot Plugin Window Class"), hInstance);
		if(Shared)
		{
			UnmapViewOfFile(Shared);
			Shared = NULL;
		}
		if(Filemap)
		{
			CloseHandle(Filemap);
			Filemap = NULL;
		}
		CloseHandle(FilemapMutex);
	}
}

HWND Dexpot::CreateCallbackWindow()
{
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = Dexpot::WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _T("Dexpot Plugin Window Class");
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(_T("Dexpot Plugin Window Class"), _T("Dexpot Plugin"),
                              0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              HWND_MESSAGE, NULL, hInstance, NULL);

    SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) &~ WS_CAPTION);
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW &~ WS_EX_APPWINDOW);
    SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);
	SendMessage(hWndDexpot, DEX_SETSWITCHINGEXCEPTION, (WPARAM) hWnd, DEX_EXCEPTION_IGNORE);

	return hWnd;
}

INT32 Dexpot::GetDesktopCount()
{
	return SendMessage(hWndDexpot, DEX_GETDESKTOPCOUNT, 0, 0);
}

INT32 Dexpot::GetCurrentDesktop()
{
	return SendMessage(hWndDexpot, DEX_GETCURRENTDESKTOP, 0, 0);
}

INT32 Dexpot::GetDesktopWidth(INT32 Desktop)
{
	return SendMessage(hWndDexpot, DEX_GETDESKTOPWIDTH, Desktop, 0);
}

INT32 Dexpot::GetDesktopHeight(INT32 Desktop)
{
	return SendMessage(hWndDexpot, DEX_GETDESKTOPHEIGHT, Desktop, 0);
}

INT32 Dexpot::SetSwitchingException(HWND hWnd, INT32 ExceptionType)
{
	return SendMessage(hWndDexpot, DEX_SETSWITCHINGEXCEPTION, (WPARAM) hWnd, ExceptionType);
}

INT32 Dexpot::GetSwitchingException(HWND hWnd)
{
	return SendMessage(hWndDexpot, DEX_GETSWITCHINGEXCEPTION, (WPARAM) hWnd, 0);
}

VOID Dexpot::SetMinAnimation(BOOL Activate)
{
	SendMessage(hWndDexpot, DEX_SETMINANIMATION, Activate, 0);
}

UINT32 Dexpot::GetDesktopString(UINT32 Message, INT32 Desktop, LPTSTR Buffer, UINT32 Size)
{
	if(Shared)
	{
		COPYDATASTRUCT cds;
		UINT32 len;

		if(FilemapMutex) WaitForSingleObject(FilemapMutex, INFINITE);

		cds.dwData = Message;
		cds.cbData = sizeof Desktop + sizeof Size;
		cds.lpData = new BYTE[cds.cbData];
		memcpy(cds.lpData, &Desktop, sizeof Desktop);
		memcpy((INT32*)cds.lpData + 1, &Size, sizeof Size);
		len = (UINT32) SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
		if(len > FILEMAP_SIZE) len = FILEMAP_SIZE;
		delete[] cds.lpData;

		if(Buffer) 
		{
            if(len > Size) len = Size;
			memcpy_s(Buffer, Size, Shared, len);
			Buffer[Size / sizeof(TCHAR) - 1] = _T('\0');
		}

		if(FilemapMutex) ReleaseMutex(FilemapMutex);
		return len;
	}

	return 0;
}

UINT32 Dexpot::GetString(UINT32 Message, LPTSTR Buffer, UINT32 Size)
{
	if(Shared)
	{
		COPYDATASTRUCT cds;
		UINT32 len;

		if(FilemapMutex) WaitForSingleObject(FilemapMutex, INFINITE);

		cds.dwData = Message;
		cds.cbData = sizeof Size;
		cds.lpData = new BYTE[cds.cbData];
		memcpy(cds.lpData, &Size, sizeof Size);
		len = (UINT32) SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
		if(len > FILEMAP_SIZE) len = FILEMAP_SIZE;
		delete[] cds.lpData;

		if(Buffer) 
		{
            if(len > Size) len = Size;
			memcpy_s(Buffer, Size, Shared, len);
			Buffer[Size / sizeof(TCHAR) - 1] = _T('\0');
		}

		if(FilemapMutex) ReleaseMutex(FilemapMutex);
		return len;
	}

	return 0;
}

UINT32 Dexpot::GetWindowsOnDesktop(INT32 Desktop, HWND *Buffer, UINT32 Size)
{
	if(Shared || (Buffer == NULL && Size == 0))
	{
		COPYDATASTRUCT cds;
		UINT32 len;
		UINT32 ByteSize = Size * sizeof(HWND);

		if(FilemapMutex) WaitForSingleObject(FilemapMutex, INFINITE);

		cds.dwData = DEX_GETWINDOWSONDESKTOP;
		cds.cbData = sizeof Desktop + sizeof ByteSize;
		cds.lpData = new BYTE[cds.cbData];
		memcpy(cds.lpData, &Desktop, sizeof Desktop);
		memcpy((INT32*)cds.lpData + 1, &ByteSize, sizeof ByteSize);
		len = (UINT32) SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
		if(len > FILEMAP_SIZE) len = FILEMAP_SIZE - (FILEMAP_SIZE % sizeof(HWND));
		delete[] cds.lpData;

		if(Buffer) 
		{
			memcpy_s(Buffer, ByteSize, Shared, len);
		}

		if(FilemapMutex) ReleaseMutex(FilemapMutex);
		return len / sizeof(HWND);
	}

	return 0;
}

INT32 Dexpot::GetNumberOfWindowsOnDesktop(INT32 Desktop)
{
	return GetWindowsOnDesktop(Desktop, NULL, 0);
}

UINT32 Dexpot::GetDesktopTitle(INT32 Desktop, LPTSTR Buffer, UINT32 Size)
{
	return GetDesktopString(DEX_GETDESKTOPTITLE, Desktop, Buffer, Size);
}

UINT32 Dexpot::GetDesktopWallpaper(INT32 Desktop, LPTSTR Buffer, UINT32 Size)
{
	return GetDesktopString(DEX_GETDESKTOPWALLPAPER, Desktop, Buffer, Size);
}

HWND Dexpot::GetActiveWindow(INT32 Desktop)
{
	return (HWND) SendMessage(hWndDexpot, DEX_GETACTIVEWINDOW, Desktop, 0);
}

UINT32 Dexpot::GetDexpotHome(LPTSTR Buffer, UINT32 Size)
{
	return GetString(DEX_GETDEXPOTHOME, Buffer, Size);
}

UINT32 Dexpot::GetAppDataPath(LPTSTR Buffer, UINT32 Size)
{
	return GetString(DEX_GETAPPDATAPATH, Buffer, Size);
}

UINT32 Dexpot::GetLanguageFile(LPTSTR Buffer, UINT32 Size)
{
	return GetString(DEX_GETLANGUAGEFILE, Buffer, Size);
}

INT32 Dexpot::GetTrayIconMode()
{
	return SendMessage(hWndDexpot, DEX_GETTRAYICONMODE, 0, 0);
}

INT32 Dexpot::SetTrayIconMode(INT32 NewMode)
{
	return SendMessage(hWndDexpot, DEX_SETTRAYICONMODE, NewMode, 0);
}

VOID Dexpot::GatherWindows(INT32 Desktop)
{
	SendNotifyMessage(hWndDexpot, DEX_GATHERWINDOWS, Desktop, 0);
}

LRESULT Dexpot::MoveWindow(HWND hWnd, INT32 Desktop)
{
	return SendMessage(hWndDexpot, DEX_MOVEWINDOW, (WPARAM) hWnd, Desktop);
}

LRESULT Dexpot::MoveWindow(HWND hWnd, INT32 TargetDesktop, INT32 SourceDesktop)
{
	return SendMessage(hWndDexpot, DEX_MOVEWINDOW, (WPARAM) hWnd, MAKELPARAM(TargetDesktop, SourceDesktop));
}

LRESULT Dexpot::CopyWindow(HWND hWnd, INT32 Desktop)
{
	return SendMessage(hWndDexpot, DEX_COPYWINDOW, (WPARAM) hWnd, Desktop);
}

VOID Dexpot::AssignWindow(HWND hWnd, INT32 Desktop)
{
	SendNotifyMessage(hWndDexpot, DEX_ASSIGNWINDOW, (WPARAM) hWnd, Desktop);
}

VOID Dexpot::SetForegroundWindow(HWND hWnd)
{
	SendNotifyMessage(hWndDexpot, DEX_SETFOREGROUNDWINDOW, (WPARAM) hWnd, 0);
}

LRESULT Dexpot::ShowWindow(HWND hWnd)
{
	return SendMessage(hWndDexpot, DEX_SHOWWINDOW, (WPARAM) hWnd, 0);
}

LRESULT Dexpot::SwitchToWindow(HWND hWnd)
{
	return SendMessage(hWndDexpot, DEX_SWITCHTOWINDOW, (WPARAM) hWnd, 0);
}

VOID Dexpot::OpenWindowMenu(HWND hWnd, INT32 Desktop)
{
	::SetForegroundWindow(hWndDexpot);
	SendNotifyMessage(hWndDexpot, DEX_OPENWINDOWMENU, (WPARAM) hWnd, Desktop);
}

VOID Dexpot::OpenMainMenu(BOOL Extended)
{
	::SetForegroundWindow(hWndDexpot);
	SendNotifyMessage(hWndDexpot, DEX_OPENMAINMENU, (WPARAM) Extended, 0);
}

VOID Dexpot::SwitchDesktop(INT32 Desktop, WORD Flags)
{
	SendNotifyMessage(hWndDexpot, DEX_SWITCHDESKTOP, MAKEWPARAM(Flags, (WORD)PluginID), Desktop);
}

LRESULT Dexpot::UpdateScreenshot(INT32 Desktop)
{
	return SendMessage(hWndDexpot, DEX_UPDATESCREENSHOT, Desktop, 0);
}

DWORD Dexpot::GetDebugMode()
{
	return SendMessage(hWndDexpot, DEX_GETDEBUGMODE, 0, 0);
}

void Dexpot::DebugOutput(LPCTSTR s)
{
	SendString(DEX_DEBUGOUTPUT, s);
}

LRESULT Dexpot::SendString(DWORD message, LPCTSTR s)
{
	COPYDATASTRUCT cds;

	cds.dwData = message;
	cds.cbData = _tcslen(s) * sizeof(TCHAR);
	cds.lpData = (LPVOID) s;

	return SendMessage(hWndDexpot, WM_COPYDATA, (WPARAM) hWndCallback, (LPARAM) &cds);
}

HINSTANCE Dexpot::GetInstance() 
{ 
	MEMORY_BASIC_INFORMATION mbi;  

	VirtualQuery(&GetInstance, &mbi, sizeof(mbi));  
	return (HINSTANCE) mbi.AllocationBase;  
}

LRESULT WINAPI Dexpot::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dexpot *dex;

	switch(msg)
    {
		case WM_DESTROY:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->Connected)
				{
					dex->Disconnect();
					if(dex->Connect()) break;
				}
				else
				{
					break;
				}
			}

		case DEX_SHUTDOWN:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnShutdown) dex->OnShutdown();
				if(dex->EventHandler) dex->EventHandler->OnShutdown();
			}
			return 0;

		case DEX_LOAD:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnLoad) dex->OnLoad();
				if(dex->EventHandler) dex->EventHandler->OnLoad();
			}
			return 0;

		case DEX_SWITCHREQUEST:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnSwitchRequest) return dex->OnSwitchRequest(LOWORD(lParam), HIWORD(lParam), LOWORD(wParam), HIWORD(wParam));
				if(dex->EventHandler) return dex->EventHandler->OnSwitchRequest(LOWORD(lParam), HIWORD(lParam), LOWORD(wParam), HIWORD(wParam));
			}
            return 0;

        case DEX_SWITCHING:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				WORD Flags = LOWORD(wParam);
				WORD Trigger =  HIWORD(wParam);
				dex = HandledPlugins[hWnd];
				if(Trigger == dex->PluginID) Flags &= ~DEX_SWITCH_NOANIMATION;
				if(dex->OnSwitch) dex->OnSwitch(LOWORD(lParam), HIWORD(lParam), Flags, Trigger);
				if(dex->EventHandler) dex->EventHandler->OnSwitch(LOWORD(lParam), HIWORD(lParam), Flags, Trigger);
			}
            return 0;

        case DEX_SWITCHED:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				WORD Flags = LOWORD(wParam);
				WORD Trigger =  HIWORD(wParam);
				dex = HandledPlugins[hWnd];
				if(Trigger == dex->PluginID) Flags &= ~DEX_SWITCH_NOANIMATION;
				if(dex->OnSwitched) dex->OnSwitched(LOWORD(lParam), HIWORD(lParam), LOWORD(wParam), HIWORD(wParam));
				if(dex->EventHandler) dex->EventHandler->OnSwitched(LOWORD(lParam), HIWORD(lParam), LOWORD(wParam), HIWORD(wParam));
			}
            return 0;

		case DEX_HOTKEY:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnHotkey) dex->OnHotkey(wParam, LOWORD(lParam), HIWORD(lParam));
				if(dex->EventHandler) dex->EventHandler->OnHotkey(wParam, LOWORD(lParam), HIWORD(lParam));
			}
            return 0;

		case DEX_CONFIGURE:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnConfigure) dex->OnConfigure();
				if(dex->EventHandler) dex->EventHandler->OnConfigure();
			}
            return 0;

		case DEX_DESKTOPCOUNTCHANGED:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnDesktopCountChanged) dex->OnDesktopCountChanged(wParam);
				if(dex->EventHandler) dex->EventHandler->OnDesktopCountChanged(wParam);
			}
            return 0;

		case DEX_MENUCOMMAND:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->OnMenuCommand) dex->OnMenuCommand(wParam);
				if(dex->EventHandler) dex->EventHandler->OnMenuCommand(wParam);
			}
            return 0;

		case DEX_DESKTOPCONTENTCHANGED:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->EventHandler) dex->EventHandler->OnDesktopContentChanged(wParam);
			}
			return 0;

		case WM_HOOXPOTRUFT:
			if(HandledPlugins.find(hWnd) != HandledPlugins.end())
			{
				dex = HandledPlugins[hWnd];
				if(dex->EventHandler)
				{
					if(lParam == HCBT_CREATEWND)
						dex->EventHandler->OnWindowCreated((HWND) wParam);
					else if(lParam == HCBT_DESTROYWND)
						dex->EventHandler->OnWindowDestroyed((HWND) wParam);
				}
			}
			return 0;

		case WM_COPYDATA:
			{
				COPYDATASTRUCT *cds = (COPYDATASTRUCT*) lParam;

				if(HandledPlugins.find(hWnd) != HandledPlugins.end())
				{
					dex = HandledPlugins[hWnd];
					if((HWND) wParam != dex->hWndDexpot) break;
					if(cds->dwData == DEX_SETOPTION && cds->cbData >= sizeof(BYTE) + sizeof(INT32))
					{
						BYTE type;
						INT32 id;
						LPVOID p = cds->lpData;
						DWORD length = cds->cbData - (sizeof(type) + sizeof(id));
						memcpy(&type, p, sizeof(type));
						p = (BYTE*)p + 1;
						memcpy(&id, p, sizeof(id));
						p = (INT32*)p + 1;
						if(type == DEX_OPTION_TYPE_INTEGER && length == sizeof(INT32))
						{
							if(dex->OnSetIntegerOption) dex->OnSetIntegerOption(id, *((INT32*)p));
							if(dex->EventHandler) dex->EventHandler->OnSetIntegerOption(id, *((INT32*)p));
						}
						else if(type == DEX_OPTION_TYPE_STRING)
						{
							TCHAR *s = new TCHAR[length / sizeof(TCHAR) + 1];
							memcpy(s, p, length);
							s[length / sizeof(TCHAR)] = _T('\0');
							if(dex->OnSetStringOption) dex->OnSetStringOption(id, s);
							if(dex->EventHandler) dex->EventHandler->OnSetStringOption(id, s);
							delete[] s;
						}
						return 0;
					}
				}
				break;
			}
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
