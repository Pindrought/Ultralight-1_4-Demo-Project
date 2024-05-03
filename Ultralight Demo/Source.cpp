//Author: Jacob Preston
//Github Repository: 
//This project is not affiliated with Ultralight. 
// Please refer to Ultralight's license/pricing as this is using the Ultralight libraries.
// Ultralight pricing: https://ultralig.ht/pricing/
// Ultralight license: https://github.com/ultralight-ux/Ultralight/blob/master/license/LICENSE.txt
//License: Please see the Ultralight license. I don't care what you do with my personal code.
//Note: This solution is currently a work in progress. There are things that are either not yet implemented or not functioning correctly.
#include <PCH.h>
#include "ErrorHandler.h"
#include "Engine.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ PWSTR pCmdLine,
					_In_ int nCmdShow)
{
	HRESULT hr = CoInitializeEx(NULL,
								COINIT_MULTITHREADED);
	FatalErrorIfFail(hr, "Failed to Initialize the COM Library. Program will now abort.");


	WindowCreationParameters parms;
	parms.Title = "Ultralight 1.4 Beta Demo 2024-04-23";
	parms.Width = 400;
	parms.Height = 200;
	parms.Style = WindowStyle::NoBorder | WindowStyle::Resizable;

	Engine engine;
	if (engine.Initialize(parms))
	{
		while (engine.IsRunning())
		{
			engine.ProcessWindowsMessages();
			engine.Tick();
		}
	}

	return 0;
}