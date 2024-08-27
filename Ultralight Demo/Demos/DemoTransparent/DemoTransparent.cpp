#include "PCH.h"
#include "DemoTransparent.h"
#include "../Misc/CursorManager.h"

bool DemoTransparent::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	//windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable | WindowStyle::TransparencyAllowed;
	windowParms.Style =  WindowStyle::NoBorder | WindowStyle::TransparencyAllowed;

	windowParms.Title = "Default Title";
	m_PrimaryWindow = SpawnWindow(windowParms);
	if (m_PrimaryWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_PrimaryWindow->GetWidth();
	parms.Height = m_PrimaryWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/TransparentWindow/TransparentWindow.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), m_PrimaryWindow->GetId());
}

EZJSParm DemoTransparent::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "CloseWindow") {
		m_PrimaryWindow->Close();
	}
	return EZJSParm();
}

void DemoTransparent::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoTransparent::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoTransparent::OnWindowResizeCallback(Window* pWindow)
{
}