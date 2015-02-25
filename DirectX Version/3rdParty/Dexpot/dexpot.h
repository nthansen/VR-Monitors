#ifndef _DEXPOT_H_
#define _DEXPOT_H_

#include <tchar.h>
#include <windows.h>
#include <map>

#include "constants.h"

class DexpotEventHandler
{
public:
	virtual VOID OnSwitch(INT32 FromDesktop, INT32 ToDesktop, WORD Flags, WORD Trigger) {};
	virtual VOID OnSwitched(INT32 FromDesktop, INT32 ToDesktop, WORD Flags, WORD Trigger) {};
	virtual VOID OnShutdown(VOID) {};
	virtual VOID OnLoad(VOID) {};
	virtual VOID OnHotkey(INT32 Id, WORD Hotkey, WORD Modifier) {};
	virtual VOID OnSetIntegerOption(INT32 Id, INT32 Value) {};
	virtual VOID OnSetStringOption(INT32 Id, LPTSTR Value) {};
	virtual VOID OnConfigure(VOID) {};
	virtual DWORD OnSwitchRequest(INT32 FromDesktop, INT32 ToDesktop, WORD Flags, WORD Trigger) {return 0;};
	virtual VOID OnDesktopCountChanged(INT32 NewDesktopCount) {};
	virtual VOID OnMenuCommand(INT32 Id) {};
	virtual VOID OnWindowCreated(HWND hWnd) {};
	virtual VOID OnWindowDestroyed(HWND hWnd) {};
	virtual VOID OnDesktopContentChanged(INT32 Desktop) {};
};

class Dexpot 
{
public:
	Dexpot(LPCTSTR Name);
	~Dexpot();

	BOOL Connect();
	VOID Disconnect();
	HWND GetDexpotWindow();
	BOOL DexpotRunning();
	DWORD GetPluginID();
	HWND GetCallbackWindow();

	VOID RegisterOnSwitchHandler(VOID (*fp)(INT32, INT32, WORD, WORD));
	VOID RegisterOnSwitchedHandler(VOID (*fp)(INT32, INT32, WORD, WORD));
	VOID RegisterOnShutdownHandler(VOID (*fp)(VOID));
	VOID RegisterOnLoadHandler(VOID (*fp)(VOID));
	VOID RegisterOnHotkeyHandler(VOID (*fp)(INT32, WORD, WORD));
	VOID RegisterOnSetIntegerOptionHandler(VOID (*fp)(INT32, INT32));
	VOID RegisterOnSetStringOptionHandler(VOID (*fp)(INT32, LPTSTR));
	VOID RegisterOnConfigureHandler(VOID (*fp)(VOID));
	VOID RegisterOnSwitchRequestHandler(DWORD (*fp)(INT32, INT32, WORD, WORD));
	VOID RegisterOnDesktopCountChangedHandler(VOID (*fp)(INT32));
	VOID RegisterOnMenuCommandHandler(VOID (*fp)(INT32));

	VOID RegisterEventHandler(DexpotEventHandler*);
	VOID SubscribeHooxpotEvents();
	VOID UnsubscribeHooxpotEvents();

	VOID RegisterHotkey(INT32 Id, LPCTSTR Name);
	VOID RegisterOption(BYTE Type, INT32 Id, LPCTSTR Name);
	VOID SetHotkeyValue(INT32 Id, INT32 Modifier, INT32 Hotkey); 
	VOID SetOptionValue(INT32 Id, INT32 Value);
	VOID SetOptionValue(INT32 Id, LPTSTR Value);
	VOID LoadSettings();
	VOID SaveSettings();

	VOID InsertMainMenuItem(INT32 Id, LPCTSTR Label, HICON Icon);
	VOID RemoveMainMenuItem(INT32 Id);

	INT32 GetDesktopCount();
	INT32 GetCurrentDesktop();
	INT32 GetDesktopWidth(INT32);
	INT32 GetDesktopHeight(INT32);
	UINT32 GetDesktopTitle(INT32 Desktop, LPTSTR Buffer, UINT32 BufferSizeInBytes);
	UINT32 GetDesktopWallpaper(INT32 Desktop, LPTSTR Buffer, UINT32 BufferSizeInBytes);
	HWND GetActiveWindow(INT32 Desktop);
	UINT32 GetWindowsOnDesktop(INT32 Desktop, HWND *Buffer, UINT32 BufferSize);
	INT32 GetNumberOfWindowsOnDesktop(INT32 Desktop);

	UINT32 GetDexpotHome(LPTSTR Buffer, UINT32 BufferSizeInBytes);
	UINT32 GetAppDataPath(LPTSTR Buffer, UINT32 BufferSizeInBytes);
	UINT32 GetLanguageFile(LPTSTR Buffer, UINT32 BufferSizeInBytes);

	INT32 GetTrayIconMode();
	INT32 SetTrayIconMode(INT32 Mode);

	INT32 GetSwitchingException(HWND hWnd);
	INT32 SetSwitchingException(HWND hWnd, INT32 ExceptionType);
	
	VOID SetMinAnimation(BOOL Enable);
	VOID SwitchDesktop(INT32 Desktop, WORD Flags);
	LRESULT UpdateScreenshot(INT32 Desktop);

	LRESULT MoveWindow(HWND hWnd, INT32 TargetDesktop);
	LRESULT MoveWindow(HWND hWnd, INT32 TargetDesktop, INT32 SourceDesktop);
	LRESULT CopyWindow(HWND hWnd, INT32 Desktop);
	VOID AssignWindow(HWND hWnd, INT32 Desktop);
	LRESULT SwitchToWindow(HWND hWnd);
	LRESULT ShowWindow(HWND hWnd);
	VOID SetForegroundWindow(HWND hWnd);
	VOID GatherWindows(INT32 Desktop);
	VOID OpenWindowMenu(HWND hWnd, INT32 Desktop);
	VOID OpenMainMenu(BOOL Extended = FALSE);
	
	DWORD GetDebugMode();
	VOID DebugOutput(LPCTSTR Text);


private:
	HWND CreateCallbackWindow();
	static LRESULT WINAPI WindowProc(HWND, UINT, WPARAM, LPARAM);
	static HINSTANCE GetInstance();
	static std::map<HWND, Dexpot*> HandledPlugins;

	UINT32 GetDesktopString(UINT32, INT32, LPTSTR, UINT32);
	UINT32 GetString(UINT32, LPTSTR, UINT32);
	LRESULT SendString(DWORD, LPCTSTR);
	VOID SetOptionValue(BYTE, INT32, LPVOID);

	VOID (*OnSwitch)(INT32, INT32, WORD, WORD);
	VOID (*OnSwitched)(INT32, INT32, WORD, WORD);
	VOID (*OnShutdown)(VOID);
	VOID (*OnLoad)(VOID);
	VOID (*OnHotkey)(INT32, WORD, WORD);
	VOID (*OnSetIntegerOption)(INT32, INT32);
	VOID (*OnSetStringOption)(INT32, LPTSTR);
	VOID (*OnConfigure)(VOID);
	DWORD (*OnSwitchRequest)(INT32, INT32, WORD, WORD);
	VOID (*OnDesktopCountChanged)(INT32);
	VOID (*OnMenuCommand)(INT32);

	DexpotEventHandler *EventHandler;

	HWND hWndCallback;
	HWND hWndDexpot;
	LPTSTR PluginName;
	DWORD PluginID;
	HINSTANCE hInstance;
	HANDLE Filemap;
	HANDLE FilemapMutex;
	LPVOID Shared;
	
	BOOL Connected;

};


#endif