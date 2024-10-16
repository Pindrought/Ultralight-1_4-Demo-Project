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
	m_MainWindow = WindowManager::SpawnWindow(mainWindowParms);
	if (m_MainWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters mainViewParms;
	mainViewParms.Name = "Main View [Google]";
	mainViewParms.Width = m_MainWindow->GetWidth();
	mainViewParms.Height = m_MainWindow->GetHeight();
	mainViewParms.IsAccelerated = false; //Use GPU Rendering?
	mainViewParms.ForceMatchWindowDimensions = true;
	mainViewParms.IsTransparent = true;

	m_MainView = m_UltralightMgr->CreateUltralightView(mainViewParms);
	if (m_MainView.expired())
	{
		FatalError("Failed to initialize view. Program must now abort.");
		return false;
	}
	m_MainView->LoadURL("http://www.google.com");
	m_UltralightMgr->SetViewToWindow(m_MainView->GetId(), m_MainWindow->GetId());

	//Inspector view creation
	WindowCreationParameters inspectorWindowParms;
	inspectorWindowParms.Width = 800;
	inspectorWindowParms.Height = 600;
	inspectorWindowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	inspectorWindowParms.Title = "Inspector Window";
	m_InspectorWindow = WindowManager::SpawnWindow(inspectorWindowParms);
	if (m_InspectorWindow.expired())
	{
		FatalError("Failed to initialize inspector window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters inspectorViewParms;
	inspectorViewParms.Name = "Inspector View";
	inspectorViewParms.Width = m_InspectorWindow->GetWidth();
	inspectorViewParms.Height = m_InspectorWindow->GetHeight();
	inspectorViewParms.IsAccelerated = false; //Use GPU Rendering?
	inspectorViewParms.ForceMatchWindowDimensions = true;
	inspectorViewParms.IsTransparent = false;
	inspectorViewParms.InspectionTarget = m_MainView;

	m_InspectorView = m_UltralightMgr->CreateUltralightView(inspectorViewParms);
	if (m_InspectorView.expired())
	{
		FatalError("Failed to initialize inspector view. Program must now abort.");
		return false;
	}
	//Note that m_UltralightMgr->CreateUltralightView will call CreateLocalInspectorView on the inspection target when that parameter is passed into the creation parms.
	m_UltralightMgr->SetViewToWindow(m_InspectorView->GetId(), m_InspectorWindow->GetId());

	return true;
}

EZJSParm DemoInspector::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoInspector::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	m_UltralightMgr->DestroyView(m_InspectorView); //Regardless of which window is destroyed first
	m_UltralightMgr->DestroyView(m_MainView);      //Going to try to delete inspector view then main view
	WindowManager::DestroyAllWindows();
}

void DemoInspector::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoInspector::OnWindowResizeCallback(Window* pWindow)
{
}

