#include "PCH.h"
#include "DemoCPPTextureInBrowser.h"

bool DemoCPPTextureInBrowser::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "C++ Texture In Browser Demo";
	WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);
	if (pWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	m_Texture = std::make_shared<Texture>();
	if (!m_Texture->Initialize(DirectoryHelper::GetAssetsDirectoryA() + "AIBowser.png"))
	{
		FatalError("Failed to load AIBowser.png.");
		return false;
	}

	IGPUDriverD3D11* pGPU = m_UltralightMgr->GetGPUDriver();
	uint32_t reservedTextureId = pGPU->RegisterCustomTextureAndReserveId(m_Texture);
	ul::Rect bounds;
	bounds.left = 0.0f;
	bounds.top = 0.0f;
	bounds.right = 1.0f;
	bounds.bottom = 1.0f; 
	ul::RefPtr<ul::ImageSource> imgSource = ul::ImageSource::CreateFromTexture(m_Texture->m_Width,
																				m_Texture->m_Height,
																				reservedTextureId,
																				bounds);

	ul::ImageSourceProvider& provider = ul::ImageSourceProvider::instance();
	provider.AddImageSource("AIBowser", imgSource);

	UltralightViewCreationParameters parms;
	parms.Width = pWindow->GetWidth();
	parms.Height = pWindow->GetHeight();
	parms.IsAccelerated = true;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;
	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/CPPTextureInBrowser/CPPTextureInBrowser.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());

	return true;
}

EZJSParm DemoCPPTextureInBrowser::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoCPPTextureInBrowser::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	pWindow->DestroyAllViewsLinkedToThisWindow();
}

void DemoCPPTextureInBrowser::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoCPPTextureInBrowser::OnWindowResizeCallback(Window* pWindow)
{
}

void DemoCPPTextureInBrowser::OnShutdown()
{
	ul::ImageSourceProvider& provider = ul::ImageSourceProvider::instance();
	provider.RemoveImageSource("AIBowser");
}
