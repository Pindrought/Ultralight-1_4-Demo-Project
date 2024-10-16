#include "PCH.h"
#include "DemoOverlayedCPPTextureOnDiv.h"
#include "../Misc/CursorManager.h"

bool DemoOverlayedCPPTextureOnDiv::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Overlayed CPP Texture Example";
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

	WeakWrapper<UltralightView> m_View = m_UltralightMgr->CreateUltralightView(parms);
	if (m_View.expired())
	{
		FatalError("Failed to initialize view. Program must now abort.");
		return false;
	}
	m_View->LoadURL("file:///Samples/OverlayedCPPTextureOnDiv/OverlayedCPPTextureOnDiv.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

	m_CPPTexture = std::make_shared<Texture>();
	if (!m_CPPTexture->Initialize(DirectoryHelper::GetAssetsDirectoryA() + "AIBowser.png"))
	{
		FatalError("Failed to load AIBowser.png.");
		return false;
	}

	return true;
}

EZJSParm DemoOverlayedCPPTextureOnDiv::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "UpdateDivRect")
	{
		assert(parameters.size() == 4);
		assert(parameters[0].GetType() == EZJSParm::Number);
		assert(parameters[1].GetType() == EZJSParm::Number);
		assert(parameters[2].GetType() == EZJSParm::Number);
		assert(parameters[3].GetType() == EZJSParm::Number);
		float x = parameters[0].AsDouble();
		float y = parameters[1].AsDouble();
		float width = parameters[2].AsDouble();
		float height = parameters[3].AsDouble();
		m_DivData.X = x;
		m_DivData.Y = y;
		m_DivData.Width = width;
		m_DivData.Height = height;
	}
	return EZJSParm();
}

void DemoOverlayedCPPTextureOnDiv::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoOverlayedCPPTextureOnDiv::OnWindowDestroyEndCallback(int32_t windowId)
{

	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoOverlayedCPPTextureOnDiv::OnWindowResizeCallback(Window* pWindow)
{
}

void DemoOverlayedCPPTextureOnDiv::OnPostRenderULViews()
{
	if (m_Window.expired())
	{
		return;
	}
	WeakWrapper<RenderTargetContainer> pRenderTarget = m_Window->GetRenderTargetContainer();
	if (pRenderTarget.expired())
	{
		return;
	}
	assert(pRenderTarget != nullptr);
	m_Renderer.ActivateRenderTarget(pRenderTarget);
	m_Renderer.DrawSprite(m_CPPTexture.get(),
						  m_DivData.X, 
						  m_DivData.Y,
						  0, //Z
						  m_DivData.Width, 
						  m_DivData.Height);
}

