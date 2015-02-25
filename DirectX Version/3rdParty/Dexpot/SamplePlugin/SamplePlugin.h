#pragma once

#include <windows.h>
#include "../dexpot.h"

class SamplePlugin : public DexpotEventHandler
{
public:
	SamplePlugin(void);
	~SamplePlugin(void);

	BOOL Initialize();
	int Run();

	virtual VOID OnSwitched(INT32 FromDesktop, INT32 ToDesktop, WORD Flags, WORD Trigger);
	virtual VOID OnShutdown(VOID);
	virtual VOID OnLoad(VOID);
	virtual VOID OnHotkey(INT32 Id, WORD Hotkey, WORD Modifier);
	virtual VOID OnSetIntegerOption(INT32 Id, INT32 Value);
	virtual VOID OnSetStringOption(INT32 Id, LPTSTR Value);
	virtual VOID OnConfigure(VOID);
	virtual VOID OnMenuCommand(INT32 Id);

private:
	Dexpot *dex;

	static const INT32 OPTION_INITIALIZED = 1;
	static const INT32 OPTION_SOMEOPTION = 2;
	static const INT32 HOTKEY_SAYHELLO = 1;
	static const INT32 MENUCOMMAND_QUIT = 1;

	int OptionsInitialized;

};
