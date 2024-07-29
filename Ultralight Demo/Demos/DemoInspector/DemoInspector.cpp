#include "PCH.h"
#include "DemoInspector.h"
#include "../Misc/CursorManager.h"

bool DemoInspector::Startup()
{
	//Main view creation
	WindowCreationParameters mainWindowParms;
	mainWindowParms.Width = 800;
	mainWindowParms.Height = 600;
	mainWindowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	mainWindowParms.Title = "Default Title";
	m_MainWindow = SpawnWindow(mainWindowParms);
	if (m_MainWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters mainViewParms;
	mainViewParms.Width = m_MainWindow->GetWidth();
	mainViewParms.Height = m_MainWindow->GetHeight();
	mainViewParms.IsAccelerated = false; //Use GPU Rendering?
	mainViewParms.ForceMatchWindowDimensions = true;
	mainViewParms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(mainViewParms);
	pView->LoadURL("http://www.google.com");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), m_MainWindow->GetId());

	//Inspector view creation
	WindowCreationParameters inspectorWindowParms;
	inspectorWindowParms.Width = 800;
	inspectorWindowParms.Height = 600;
	inspectorWindowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	inspectorWindowParms.Title = "Inspector Window";
	m_InspectorWindow = SpawnWindow(inspectorWindowParms);
	if (m_InspectorWindow == nullptr)
	{
		FatalError("Failed to initialize inspector window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters inspectorViewParms;
	inspectorViewParms.Width = m_InspectorWindow->GetWidth();
	inspectorViewParms.Height = m_InspectorWindow->GetHeight();
	inspectorViewParms.IsAccelerated = false; //Use GPU Rendering?
	inspectorViewParms.ForceMatchWindowDimensions = true;
	inspectorViewParms.IsTransparent = false;
	inspectorViewParms.InspectionTarget = pView;

	shared_ptr<UltralightView> pInspectorView = m_UltralightMgr->CreateUltralightView(inspectorViewParms);
	//Note that m_UltralightMgr->CreateUltralightView will call CreateLocalInspectorView on the inspection target when that parameter is passed into the creation parms.
	m_UltralightMgr->SetViewToWindow(pInspectorView->GetId(), m_InspectorWindow->GetId());

	return true;

}

EZJSParm DemoInspector::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoInspector::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
	if (m_MainWindow != nullptr)
	{
		if (m_MainWindow->GetId() == windowId)
		{
			m_MainWindow = nullptr;
			if (m_InspectorWindow != nullptr)
			{
				m_InspectorWindow->Close();
			}
		}
	}
	if (m_InspectorWindow != nullptr)
	{
		if (m_InspectorWindow->GetId() == windowId)
		{
			m_InspectorWindow = nullptr;
			if (m_MainWindow != nullptr)
			{
				m_MainWindow->Close();
			}
		}
	}
}

void DemoInspector::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoInspector::OnWindowResizeCallback(Window* pWindow)
{
}

