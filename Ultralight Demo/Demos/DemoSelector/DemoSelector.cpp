#include "PCH.h"
#include "DemoSelector.h"
#include "../Misc/CursorManager.h"

bool DemoSelector::Startup()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	WindowCreationParameters windowParms;
	windowParms.Width = screenWidth - 100;
	windowParms.Height = std::min(720, screenHeight);
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo Selector";
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
	m_View->LoadURL("file:///Samples/DemoSelector/DemoSelector.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

	return true;
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
			m_Window->Close();
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

