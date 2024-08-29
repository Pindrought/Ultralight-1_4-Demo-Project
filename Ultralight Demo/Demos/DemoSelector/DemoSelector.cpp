#include "PCH.h"
#include "DemoSelector.h"
#include "../Misc/CursorManager.h"

bool DemoSelector::Startup()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	WindowCreationParameters windowParms;
	windowParms.Width = screenWidth - 100;
	windowParms.Height = screenHeight - 100;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo Selector";
	m_PrimaryWindow = WindowManager::SpawnWindow(windowParms);
	if (m_PrimaryWindow.expired())
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

	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/DemoSelector/DemoSelector.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), m_PrimaryWindow->GetId());
}

EZJSParm DemoSelector::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "SelectDemo")
	{
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::String);
		string demoName = parameters[0].AsString();
		if (demoName == "DemoAntiAliasTest")
		{
			m_SelectedDemo = DemoId::DemoAntiAliasTest;
		}
		if (demoName == "DemoBasic")
		{
			m_SelectedDemo = DemoId::DemoBasic;
		}
		if (demoName == "DemoBorderlessResizable")
		{
			m_SelectedDemo = DemoId::DemoBorderlessResizable;
		}
		if (demoName == "DemoBorderlessResizableMovable")
		{
			m_SelectedDemo = DemoId::DemoBorderlessResizableMovable;
		}
		if (demoName == "DemoCPPTextureInBrowser")
		{
			m_SelectedDemo = DemoId::DemoCPPTextureInBrowser;
		}
		if (demoName == "DemoCubeDraw")
		{
			m_SelectedDemo = DemoId::DemoCubeDraw;
		}
		if (demoName == "DemoInspector")
		{
			m_SelectedDemo = DemoId::DemoInspector;
		}
		if (demoName == "DemoJSCPPCommunication")
		{
			m_SelectedDemo = DemoId::DemoJSCPPCommunication;
		}
		if (demoName == "DemoOpenFileDialog")
		{
			m_SelectedDemo = DemoId::DemoOpenFileDialog;
		}
		if (demoName == "DemoTransparent")
		{
			m_SelectedDemo = DemoId::DemoTransparent;
		}
		if (demoName == "DemoGLTFViewer")
		{
			m_SelectedDemo = DemoId::DemoGLTFViewer;
		}
		if (demoName == "DemoOverlayedCPPTexture")
		{
			m_SelectedDemo = DemoId::DemoOverlayedCPPTexture;
		}
		if (demoName == "DemoMultipleCubesEmbedded")
		{
			m_SelectedDemo = DemoId::DemoMultipleCubesEmbedded;
		}
		if (m_SelectedDemo != DemoId::None)
		{
			m_PrimaryWindow->Close();
		}
	}
	return EZJSParm();
}

void DemoSelector::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoSelector::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoSelector::OnWindowResizeCallback(Window* pWindow)
{
}

