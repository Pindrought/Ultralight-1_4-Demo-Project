#include "PCH.h"
#include "DemoCPPTextureInBrowser.h"

bool DemoCPPTextureInBrowser::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "C++ Texture In Browser Demo";
	m_Window = WindowManager::SpawnWindow(windowParms);
	if (m_Window.expired())
	{
		FatalError("Failed to initialize window. Program must now abort.");
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
	parms.Width = m_Window->GetWidth();
	parms.Height = m_Window->GetHeight();
	parms.IsAccelerated = true;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;
	m_View = m_UltralightMgr->CreateUltralightView(parms);
	if (m_View.expired())
	{
		FatalError("Failed to initialize view. Program must now abort.");
		return false;
	}
	m_View->LoadURL("file:///Samples/CPPTextureInBrowser/CPPTextureInBrowser.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

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

DemoCPPTextureInBrowser::~DemoCPPTextureInBrowser()
{
	ul::ImageSourceProvider& provider = ul::ImageSourceProvider::instance();
	ul::RefPtr<ul::ImageSource> imgSrc = provider.GetImageSource("AIBowser");
	if (imgSrc.get() != nullptr)
	{
		IGPUDriverD3D11* pDriver = m_UltralightMgr->GetGPUDriver();
		pDriver->DestroyTexture(imgSrc->texture_id());
		provider.RemoveImageSource("AIBowser");
	}
}
