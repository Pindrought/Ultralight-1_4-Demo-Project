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
#include "Demos/Demos.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
					_In_opt_ HINSTANCE hPrevInstance,
					_In_ PWSTR pCmdLine,
					_In_ int nCmdShow)
{
	HRESULT hr = CoInitializeEx(NULL,
								COINIT_MULTITHREADED);
	FatalErrorIfFail(hr, "Failed to Initialize the COM Library. Program will now abort.");

	while (true)
	{
		DemoScreenShareHelper engine;
		if (engine.Initialize())
		{
			while (engine.IsRunning())
			{
				engine.ProcessWindowsMessages();
				engine.Tick();
				engine.RenderFrame();
			}
		}
		break;
		//DemoSelector::DemoId demoId = DemoSelector::DemoId::None;
		//{
		//	DemoSelector engine;
		//	if (engine.Initialize())
		//	{

		//		while (engine.IsRunning())
		//		{
		//			engine.ProcessWindowsMessages();
		//			engine.Tick();
		//			engine.RenderFrame();
		//		}
		//	}
		//	else
		//	{
		//		FatalError("Error occurred starting up demo selector demo. Application must close.");
		//	}
		//	demoId = engine.m_SelectedDemo;
		//}


		//shared_ptr<Engine> demoInstance = GenerateEngineInstanceForDemo(demoId);

		//if (demoInstance == nullptr) //No demo was selected if this is nullptr, so we exit
		//{
		//	break;
		//}

		//if (demoInstance->Initialize())
		//{
		//	while (demoInstance->IsRunning())
		//	{
		//		demoInstance->ProcessWindowsMessages();
		//		demoInstance->Tick();
		//		demoInstance->RenderFrame();
		//	}
		//}
		//else
		//{
		//	FatalError("Error starting demo. Application must close.");
		//}

	}

	return 0;
}