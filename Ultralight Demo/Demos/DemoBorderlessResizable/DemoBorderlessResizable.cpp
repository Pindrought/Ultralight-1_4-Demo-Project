#include "PCH.h"
#include "DemoBorderlessResizable.h"
#include "../Misc/CursorManager.h"

bool DemoBorderlessResizable::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::NoBorder;
	windowParms.Title = "Default Title";
	m_Window = WindowManager::SpawnWindow(windowParms);
	if (m_Window.expired())
	{
		FatalError("Failed to initialize window. Program must now abort.");
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
	m_View->LoadURL("file:///Samples/BorderlessWindow/BorderlessWindow.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

	return true;
}

EZJSParm DemoBorderlessResizable::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	WeakWrapper<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);
	assert(!pView.expired());

	if (eventName == "RequestTitle") //BorderlessWindow.html calls this on load to fill span for title
	{
		return "BorderlessResizable Demo";
	}

	if (eventName == "CloseWindow")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
		pWindow->Close();
		return nullptr;
	}

	if (eventName == "MaximizeRestore")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
		if (pWindow->IsWindowMaximized())
		{
			pWindow->Restore();
		}
		else
		{
			pWindow->Maximize();
		}
		return nullptr;
	}

	return EZJSParm();
}

void DemoBorderlessResizable::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	pWindow->DestroyAllViewsLinkedToThisWindow();
}

void DemoBorderlessResizable::OnWindowDestroyEndCallback(int32_t windowId)
{
	SetRunning(false);
}

void DemoBorderlessResizable::OnWindowResizeCallback(Window* pWindow)
{
}