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
	m_Window = WindowManager::SpawnWindow(windowParms);
	if (m_Window.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_Window->GetWidth();
	parms.Height = m_Window->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	m_View = m_UltralightMgr->CreateUltralightView(parms);
	if (m_View.expired())
	{
		FatalError("Failed to initialize view. Program must now abort.");
		return false;
	}
	m_View->LoadURL("file:///Samples/TransparentWindow/TransparentWindow.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

	return true;
}

EZJSParm DemoTransparent::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "CloseWindow") {
		m_Window->Close();
	}
	return EZJSParm();
}

void DemoTransparent::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoTransparent::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoTransparent::OnWindowResizeCallback(Window* pWindow)
{
}