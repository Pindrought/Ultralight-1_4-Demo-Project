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
		DemoSelector::DemoId demoId = DemoSelector::DemoId::None;
		{
			DemoSelector engine;
			if (engine.Initialize())
			{
				while (engine.IsRunning())
				{
					engine.ProcessWindowsMessages();
					engine.Tick();
					engine.RenderFrame();
				}
			}
			demoId = engine.m_SelectedDemo;
		}

		shared_ptr<Engine> demoEngine = nullptr;
		switch (demoId)
		{
			case DemoSelector::DemoId::DemoBasic:
				demoEngine = make_shared<DemoBasic>();
				break;
			case DemoSelector::DemoId::DemoBorderlessResizable:
				demoEngine = make_shared<DemoBorderlessResizable>();
				break;
			case DemoSelector::DemoId::DemoBorderlessResizableMovable:
				demoEngine = make_shared<DemoBorderlessResizableMovable>();
				break;
			/*case DemoSelector::DemoId::DemoCPPTextureInBrowser: This is currently broken - waiting on Ultralight update for custom texture support via img
				demoEngine = make_shared<DemoCPPTextureInBrowser>();
				break;*/
			case DemoSelector::DemoId::DemoInspector:
				demoEngine = make_shared<DemoInspector>();
				break;
			case DemoSelector::DemoId::DemoJSCPPCommunication:
				demoEngine = make_shared<DemoJSCPPCommunication>();
				break;
			case DemoSelector::DemoId::DemoOpenFileDialog:
				demoEngine = make_shared<DemoOpenFileDialog>();
				break;
			case DemoSelector::DemoId::DemoTransparent:
				demoEngine = make_shared<DemoTransparent>();
				break;
			case DemoSelector::DemoId::DemoOverlayedCPPTexture:
				demoEngine = make_shared<DemoOverlayedCPPTextureOnDiv>();
				break;
		}

		if (demoEngine == nullptr)
		{
			break;
		}

		if (demoEngine->Initialize())
		{
			while (demoEngine->IsRunning())
			{
				demoEngine->ProcessWindowsMessages();
				demoEngine->Tick();
				demoEngine->RenderFrame();
			}
		}

	}

	return 0;
}