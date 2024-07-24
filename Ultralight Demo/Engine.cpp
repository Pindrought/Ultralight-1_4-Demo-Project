#include <PCH.h>
#include "Engine.h"
#include "Graphics/Renderer.h"
#include "Window/Window.h"
#include "Window/InputController/InputController.h"
#include "Misc/CursorManager.h"
#include "UltralightImpl/GPUimpl/GPUDriverD3D11.h"

Engine* Engine::s_Instance = nullptr;

Engine* Engine::GetInstance()
{
	return s_Instance;
}

InputController* Engine::GetInputController()
{
	return &m_InputController;
}

bool Engine::Initialize()
{
	if (s_Instance != nullptr)
	{
		FatalError("Engine already initialized. Engine should only ever be initialized once. Program must now abort.");
		return false;
	}

	s_Instance = this;
	CursorManager::Initialize(); //Used by ultralight for assigning cursor

	if (!m_Renderer.Initialize())
	{
		FatalError("Failed to initialize renderer. Program must now abort.");
		return false;
	}

	m_UltralightMgr = UltralightManager::GetInstanceShared();
	if (!InitializeUltralight())
	{
		FatalError("Ultralight initialization failed.");
		return false;
	}

	if (Startup() == false)
	{
		return false;
	}

	SetRunning(true);

    return true;
}

bool Engine::InitializeUltralight()
{
	if (!m_UltralightMgr->Initialize())
	{
		FatalError("Failed to initialize Ultralight. Program must now abort.");
		return false;
	}
	return true;
}

bool Engine::IsRunning()
{
    return m_IsRunning;
}

void Engine::ProcessWindowsMessages()
{
	//Process windows messages
	MSG msg = { 0 };
	while (PeekMessage(&msg, //Where to store message (if one exists) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
					   NULL, //Handle to window we are checking messages for (NULL = all messages)
					   0,    //Minimum Filter Msg Value - We are not filtering for specific messages, but the min/max could be used to filter only mouse messages for example.
					   0,    //Maximum Filter Msg Value
					   PM_REMOVE))//Remove message after capturing it via PeekMessage. For more argument options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644943(v=vs.85).aspx
	{
		TranslateMessage(&msg); //Translate message from virtual key messages into character messages so we can dispatch the message. See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644955(v=vs.85).aspx
		DispatchMessage(&msg); //Dispatch message to our Window Proc for this window. See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms644934(v=vs.85).aspx
	}
}

bool Engine::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Default Title";
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	if (pWindow == nullptr)
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
	pView->LoadURL("http://www.google.com");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
}

bool Engine::Tick()
{
	//Process Input Events
	auto& keyboard = m_InputController.m_Keyboard;
	auto& mouse = m_InputController.m_Mouse;
	while (mouse.EventBufferIsEmpty() == false)
	{
		MouseEvent mouseEvent = mouse.ReadEvent();
		bool dispatchedToHtml = m_UltralightMgr->FireMouseEvent(&mouseEvent);
		if (dispatchedToHtml == false) //Because of the way the window is being initialized (without a default cursor), it is
		{							   //possible to have the cursor state changed ex. resize border on window and have it not be
									   //changed back to normal if not hovering over an Ultralight View to reset it
			CursorType cursor = CursorManager::GetCursor();
			if (mouseEvent.GetType() == MouseEvent::Type::MouseMove)
			{
				//TODO: Maybe add error checking?
				
				Window* pWindow = GetWindowFromId(mouseEvent.GetWindowId());
				uint16_t windowWidth = pWindow->GetWidth();
				uint16_t windowHeight = pWindow->GetHeight();
				if (mouseEvent.GetPosX() < windowWidth && 
					mouseEvent.GetPosY() < windowHeight)
				{
					CursorManager::SetCursor(CursorType::ARROW);
					cursor = CursorType::ARROW;
				}
			}
		}
	}
	while (mouse.ScrollEventBufferIsEmpty() == false)
	{
		ScrollEvent scrollEvent = mouse.ReadScrollEvent();
		bool dispatchedToHtml = m_UltralightMgr->FireScrollEvent(&scrollEvent);
	}
	while (keyboard.EventBufferIsEmpty() == false)
	{
		KeyboardEvent keyboardEvent = keyboard.ReadEvent();
		bool dispatchedtoHtml = m_UltralightMgr->FireKeyboardEvent(&keyboardEvent);
	}

	return true;
}

void Engine::SetRunning(bool running)
{
	m_IsRunning = running;
}

shared_ptr<Window> Engine::SpawnWindow(const WindowCreationParameters& parms)
{
	std::shared_ptr<Window> window = std::make_shared<Window>();
	if (!window->Initialize(parms))
	{
		return nullptr;
	}

	m_WindowIdToWindowInstanceMap[window->GetId()] = window;
	m_UltralightMgr->RegisterWindow(window);

	return window;
}

bool Engine::CleanupWindow(int32_t windowId)
{
	//TODO: Error checking
	m_WindowIdToWindowInstanceMap.erase(windowId);
	return true;
}

Window* Engine::GetWindowFromId(int32_t windowId)
{
	//TODO: Maybe add some error checking to just crash if not found
	auto iter = m_WindowIdToWindowInstanceMap.find(windowId);
	if (iter != m_WindowIdToWindowInstanceMap.end())
	{
		return iter->second.get();
	}
	FatalError("Window not found in call to GetWindowFromId()");
	return nullptr;
}

Engine::~Engine()
{
	if (s_Instance == this)
	{
		s_Instance = nullptr;
	}
	m_WindowIdToWindowInstanceMap.clear();
	m_UltralightMgr->Shutdown();
	/*RetargetableGPUDriverD3D11* pR = (RetargetableGPUDriverD3D11*)m_UltralightMgr->GetGPUDriver();
	GPUDriverD3D11* pDriver = (GPUDriverD3D11*)pR->m_CurrentGPUDriverImpl.get();*/
	//for (int i = 0; i < 60; i++)
	//{
	//	Sleep(1);
	//	RenderFrame();
	//}
}

void Engine::RenderFrame()
{
	m_UltralightMgr->UpdateViews();

	float deltaTime = 0;
	vector<Window*> windowsFlaggedForRender;

	for (auto& windowPair : m_WindowIdToWindowInstanceMap)
	{
		int32_t windowId = windowPair.first;
		std::shared_ptr<Window> pWindow = windowPair.second;
		RenderTargetContainer* pRenderTargetContainer = pWindow->GetRenderTargetContainer();
		if (!pRenderTargetContainer->IsActive())
		{
			continue;
		}

		pRenderTargetContainer->AdvanceUpdateInterval(deltaTime);
		if (pRenderTargetContainer->IsReadyForRender())
		{
			pRenderTargetContainer->ResetReadyForRender();
			windowsFlaggedForRender.push_back(pWindow.get());
		}
		else
		{
			continue; //Do not render render targets if they are not ready to be rendered. This only happens when limiting refresh rate at render target level.
		}

		m_Renderer.ClearRenderTarget(pRenderTargetContainer);
	}

	OnPreRenderULViews(); //for if we want to draw anything before the UL views have been rendered

	for (auto pWindow : windowsFlaggedForRender) //Render each of our ultralight views to each window
	{
		for (shared_ptr<UltralightView> pUltralightView : pWindow->GetSortedUltralightViews())
		{
			m_Renderer.RenderUltralightView(pUltralightView.get());
		}
	}

	OnPostRenderULViews(); //for if we want to draw anything after the UL views have been rendered

	//Present each window's render target
	for (auto pWindow : windowsFlaggedForRender)
	{
		RenderTargetContainer* pRenderTargetContainer = pWindow->GetRenderTargetContainer();
		pRenderTargetContainer->ResolveIfNecessary();

		auto pSwapChain = pWindow->GetSwapChainPtr();
		if (m_VSync == false)
		{
			if (m_Renderer.GetD3D()->IsTearingSupported())
			{
				pSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
			}
			else
			{
				pSwapChain->Present(0, NULL);
			}
		}
		else
		{
			pSwapChain->Present(1, NULL);
		}
	}

	m_UltralightMgr->RefreshViewDisplaysForAnimations();
}

void Engine::OnPreRenderULViews()
{
}

void Engine::OnPostRenderULViews()
{
}
