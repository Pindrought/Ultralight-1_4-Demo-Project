#include "PCH.h"
#include "DemoCrashTestSelect.h"
#include "../Misc/CursorManager.h"

bool DemoCrashTestSelect::Startup()
{
	string url = "file:///Samples/CrashTestSelect/CrashTestSelect.html";

	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Crash Test";
	WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);
	if (pWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = pWindow->GetWidth();
	parms.Height = pWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = false;
	parms.SampleCount = 1;

	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL(url);
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
}

EZJSParm DemoCrashTestSelect::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoCrashTestSelect::OnWindowDestroyStartCallback(int32_t windowId)
{
	m_UltralightMgr->DestroyAllViews();
	WindowManager::DestroyAllWindows();
}

void DemoCrashTestSelect::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoCrashTestSelect::OnWindowResizeCallback(Window* pWindow)
{
}

