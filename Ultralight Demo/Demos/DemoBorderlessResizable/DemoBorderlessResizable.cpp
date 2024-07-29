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
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	m_PrimaryWindow = pWindow;
	if (m_PrimaryWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = pWindow->GetWidth();
	parms.Height = pWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/BorderlessWindow/BorderlessWindow.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
}

EZJSParm DemoBorderlessResizable::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	shared_ptr<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);
	assert(pView != nullptr);

	if (eventName == "RequestTitle") //BorderlessWindow.html calls this on load to fill span for title
	{
		return "BorderlessResizable Demo";
	}

	if (eventName == "CloseWindow")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		Window* pWindow = GetWindowFromId(windowId);
		pWindow->Close();
		return nullptr;
	}

	if (eventName == "MaximizeRestore")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		Window* pWindow = GetWindowFromId(windowId);
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
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoBorderlessResizable::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoBorderlessResizable::OnWindowResizeCallback(Window* pWindow)
{
	/*for (auto ulView : pWindow->GetSortedUltralightViews())
	{
		EZJSParm outReturnVal;
		string outException;
		bool result = ulView->CallJSFnc("UpdateDimensions",
										{ pWindow->GetWidth(), pWindow->GetHeight() },
										outReturnVal,
										outException);
		if (result == false)
		{
			ErrorHandler::LogCriticalError(outException);
		}
	}*/
}