#include "PCH.h"
#include "DemoCPPTextureInBrowser.h"

bool DemoCPPTextureInBrowser::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Overlayed CPP Texture Example";
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	if (pWindow == nullptr)
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
	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
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
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoCPPTextureInBrowser::OnWindowDestroyEndCallback(int32_t windowId)
{

	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoCPPTextureInBrowser::OnWindowResizeCallback(Window* pWindow)
{
}