#include "PCH.h"
#include "DemoBasic.h"
#include "../Misc/CursorManager.h"

bool DemoBasic::Startup()
{
	string url = "http://www.google.com";
	
	//GPU Renderer Window/View
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "GPU Renderer";
	m_WindowForGPUView = WindowManager::SpawnWindow(windowParms);
	if (m_WindowForGPUView.expired())
	{
		FatalError("Failed to initialize window for GPU view. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_WindowForGPUView->GetWidth();
	parms.Height = m_WindowForGPUView->GetHeight();
	parms.IsAccelerated = true; //Use GPU Renderer
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = false;
	parms.SampleCount = 8;

	m_GPUView = m_UltralightMgr->CreateUltralightView(parms);
	if (m_GPUView.expired())
	{
		return false;
	}
	m_GPUView->LoadURL(url);
	m_UltralightMgr->SetViewToWindow(m_GPUView->GetId(), m_WindowForGPUView->GetId());

	//CPU Renderer Window/View
	{
		WindowCreationParameters windowParms;
		windowParms.Width = 800;
		windowParms.Height = 600;
		windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
		windowParms.Title = "CPU Renderer";
		m_WindowForCPUView = WindowManager::SpawnWindow(windowParms);
		if (m_WindowForCPUView.expired())
		{
			FatalError("Failed to initialize window for CPU view. Program must now abort.");
			return false;
		}

		UltralightViewCreationParameters parms;
		parms.Width = m_WindowForCPUView->GetWidth();
		parms.Height = m_WindowForCPUView->GetHeight();
		parms.IsAccelerated = false; //Use CPU Renderer
		parms.ForceMatchWindowDimensions = true;
		parms.IsTransparent = true;

		m_CPUView = m_UltralightMgr->CreateUltralightView(parms);
		if (m_CPUView.expired())
		{
			return false;
		}
		m_CPUView->LoadURL(url);
		m_UltralightMgr->SetViewToWindow(m_CPUView->GetId(), m_WindowForCPUView->GetId());
	}

}

EZJSParm DemoBasic::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{	
	return EZJSParm();
}

void DemoBasic::OnWindowDestroyStartCallback(int32_t windowId)
{
	m_UltralightMgr->DestroyAllViews();
	WindowManager::DestroyAllWindows();
}

void DemoBasic::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoBasic::OnWindowResizeCallback(Window* pWindow)
{
}

