#include "PCH.h"
#include "DemoBasic.h"
#include "../Misc/CursorManager.h"

bool DemoBasic::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "GPU Renderer";
	WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);
	if (pWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = pWindow->GetWidth();
	parms.Height = pWindow->GetHeight();
	parms.IsAccelerated = true;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;
	parms.SampleCount = 8;

	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("http://www.google.com");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());

	{
		WindowCreationParameters windowParms;
		windowParms.Width = 800;
		windowParms.Height = 600;
		windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
		windowParms.Title = "CPU Renderer";
		WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);

		UltralightViewCreationParameters parms;
		parms.Width = pWindow->GetWidth();
		parms.Height = pWindow->GetHeight();
		parms.IsAccelerated = false;
		parms.ForceMatchWindowDimensions = true;
		parms.IsTransparent = true;

		WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
		pView->LoadURL("http://www.google.com");
		m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
	}

}

EZJSParm DemoBasic::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{	
	return EZJSParm();
}

void DemoBasic::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	pWindow->DestroyAllViewsLinkedToThisWindow();
}

void DemoBasic::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoBasic::OnWindowResizeCallback(Window* pWindow)
{
}

