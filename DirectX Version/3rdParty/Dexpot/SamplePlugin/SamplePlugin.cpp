#include "SamplePlugin.h"

#include <string>
#include <tchar.h>


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	SamplePlugin app;

	if(app.Initialize())
	{
		return app.Run();
	}

	return 0;
}

BOOL SamplePlugin::Initialize()
{
	if(!dex)
	{
		dex = new Dexpot(_T("SamplePlugin"));
		dex->RegisterEventHandler(this);
	}

	if(dex->Connect())
	{
		return TRUE;
	}

	return FALSE;
}

int SamplePlugin::Run()
{
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

void SamplePlugin::OnLoad()
{
	TCHAR DexpotPath[MAX_PATH];
	TCHAR Message[2 * MAX_PATH];
	
	dex->GetDexpotHome(DexpotPath, MAX_PATH * sizeof(TCHAR));
	_stprintf_s(Message, 2 * MAX_PATH, _T("Dexpot sample plugin has loaded.\nHere are some facts:\n\nDesktop count: %i\nCurrent desktop: %i\nDexpot path: %s"),
		        dex->GetDesktopCount(), dex->GetCurrentDesktop(), DexpotPath);
	MessageBox(NULL, Message, _T("Dexpot Sample Plugin"), MB_OK | MB_ICONINFORMATION);

	dex->RegisterOption(DEX_OPTION_TYPE_INTEGER, OPTION_INITIALIZED, _T("Initialized"));
	dex->RegisterOption(DEX_OPTION_TYPE_STRING, OPTION_SOMEOPTION, _T("SomeOption"));

	dex->RegisterHotkey(HOTKEY_SAYHELLO, _T("Say Hello"));

	dex->LoadSettings();

	if(!OptionsInitialized)
	{
		OptionsInitialized = 1;

		dex->SetOptionValue(OPTION_INITIALIZED, OptionsInitialized);
		dex->SetOptionValue(OPTION_SOMEOPTION, _T("Default value of SomeOption"));
		dex->SetHotkeyValue(HOTKEY_SAYHELLO, MOD_WIN, VK_F8);

		dex->SaveSettings();
	}

	dex->InsertMainMenuItem(MENUCOMMAND_QUIT, _T(" Quit Sample Plugin"), NULL);
}

void SamplePlugin::OnSetIntegerOption(INT32 Id, INT32 Value)
{
	if(Id == OPTION_INITIALIZED)
	{
		OptionsInitialized = Value;
	}
}

void SamplePlugin::OnSetStringOption(INT32 Id, LPTSTR Value)
{
	if(Id == OPTION_SOMEOPTION)
	{
		std::wstring SomeOption = _T("Value of SomeOption: ");
		SomeOption += std::wstring(Value);
		MessageBox(NULL, SomeOption.c_str(), _T("Dexpot Sample Plugin"), MB_OK);
	}
}

void SamplePlugin::OnHotkey(INT32 Id, WORD Hotkey, WORD Modifier)
{
	if(Id == HOTKEY_SAYHELLO)
	{
		MessageBox(NULL, _T("Hello!"), _T("Dexpot Sample Plugin"), MB_OK | MB_ICONINFORMATION);
	}
}

void SamplePlugin::OnMenuCommand(INT32 Id)
{
	if(Id == MENUCOMMAND_QUIT)
	{
		OnShutdown();
	}
}

void SamplePlugin::OnShutdown()
{
	dex->Disconnect();
	PostQuitMessage(0);
}

void SamplePlugin::OnSwitched(INT32 FromDesktop, INT32 ToDesktop, WORD Flags, WORD Trigger)
{
	if(Trigger != dex->GetPluginID())
	{
		TCHAR Buffer[128];

		dex->GetDesktopTitle(FromDesktop, Buffer, 128 * sizeof(TCHAR));
		std::wstring FromDesktopName = Buffer;

		dex->GetDesktopTitle(ToDesktop, Buffer, 128 * sizeof(TCHAR));
		std::wstring ToDesktopName = Buffer;

		std::wstring message = _T("");
		message += _T("You have switched from ") + FromDesktopName + _T(" to ") + ToDesktopName + _T(".");
		message += _T("\nWanna switch back?");
		if(MessageBox(NULL, message.c_str(), _T("Dexpot Sample Plugin"), MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
		{
			dex->SwitchDesktop(FromDesktop, 0);
		}
	}
}

void SamplePlugin::OnConfigure()
{
	MessageBox(NULL, _T("Insert plugin settings dialog here."), _T("Dexpot Sample Plugin"), MB_OK);
}

SamplePlugin::SamplePlugin(void)
{
	dex = NULL;
}

SamplePlugin::~SamplePlugin(void)
{
	delete dex;
}
