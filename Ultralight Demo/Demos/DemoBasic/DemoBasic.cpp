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
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	if (pWindow == nullptr)
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

	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("http://www.google.com");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());

	{
		WindowCreationParameters windowParms;
		windowParms.Width = 800;
		windowParms.Height = 600;
		windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
		windowParms.Title = "CPU Renderer";
		shared_ptr<Window> pWindow = SpawnWindow(windowParms);

		UltralightViewCreationParameters parms;
		parms.Width = pWindow->GetWidth();
		parms.Height = pWindow->GetHeight();
		parms.IsAccelerated = false;
		parms.ForceMatchWindowDimensions = true;
		parms.IsTransparent = true;

		shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
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
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoBasic::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoBasic::OnWindowResizeCallback(Window* pWindow)
{
}

