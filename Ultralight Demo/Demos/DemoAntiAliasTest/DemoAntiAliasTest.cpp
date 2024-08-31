#include "PCH.h"
#include "DemoAntiAliasTest.h"
#include "../Misc/CursorManager.h"

bool DemoAntiAliasTest::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 330;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	
	UltralightViewCreationParameters parms;
	parms.Width = windowParms.Width;
	parms.Height = windowParms.Height;
	parms.IsAccelerated = true;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	for (int sampleCount = 1; sampleCount <= 8; sampleCount *= 2)
	{
		if (sampleCount > 1)
		{
			if (m_Renderer.GetD3D()->IsSampleCountSupported(sampleCount) == false)
			{
				continue;
			}
		}

		windowParms.Title = "SampleCount = ";
		windowParms.Title += std::to_string(sampleCount);

		WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);
		if (pWindow.expired())
		{
			FatalError("Failed to initialize window. Program must now abort.");
			return false;
		}

		parms.SampleCount = sampleCount;
		WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
		pView->LoadURL("file:///Samples/AntiAliasTest/AntiAliasTest.html");
		m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
		m_Windows.push_back(pWindow);
	}

	//This is all to just reposition the windows so they are not stacked on top of each other
	int windowCount = m_Windows.size();
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int screenCenterX = screenWidth / 2;
	int screenCenterY = screenHeight / 2;

	for (int i = 0; i < m_Windows.size(); i++)
	{
		int xOffset = 0;
		if (i % 2 == 0)
		{
			xOffset = -(int)parms.Width;
		}

		int yOffset = 0;
		if (i < 2)
		{
			yOffset = -(int)(parms.Height + 30);
		}
		m_Windows[i]->SetPosition(screenCenterX + xOffset,
								  screenCenterY + yOffset);
	}

}

EZJSParm DemoAntiAliasTest::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoAntiAliasTest::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	pWindow->DestroyAllViewsLinkedToThisWindow();
	WindowManager::DestroyAllWindows();
}

void DemoAntiAliasTest::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoAntiAliasTest::OnWindowResizeCallback(Window* pWindow)
{
}

