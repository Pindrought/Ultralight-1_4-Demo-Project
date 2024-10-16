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

	m_OffScreenRenderTargetContainers.clear();

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

	m_FrameTimer.Start();

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

bool Engine::Tick()
{
	m_DeltaTime = m_FrameTimer.GetMilisecondsElapsed();
	m_FrameTimer.Restart();

	if (!TickStart())
	{
		return false;
	}
	if (!ProcessInput())
	{
		return false;
	}
	if (!TickEnd())
	{
		return false;
	}
	return true;
}

bool Engine::ProcessInput()
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

				WeakWrapper<Window> pWindow = WindowManager::GetWindow(mouseEvent.GetWindowId());
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

Engine::~Engine()
{
	if (s_Instance == this)
	{
		s_Instance = nullptr;
	}
	m_UltralightMgr->Shutdown();
}

void Engine::RenderFrame()
{
	OnPreRenderFrame();

	m_UltralightMgr->UpdateViews();

	vector<WeakWrapper<Window>> windowsFlaggedForRender;
	vector<WeakWrapper<RenderTargetContainer>> renderTargetsFlaggedForRender;

	auto& windowMap = WindowManager::GetWindowMap();
	for (auto& windowPair : windowMap)
	{
		int32_t windowId = windowPair.first;
		std::shared_ptr<Window> pWindow = windowPair.second;
		WeakWrapper<RenderTargetContainer> pRenderTargetContainer = pWindow->GetRenderTargetContainer();
		if (!pRenderTargetContainer->IsActive())
		{
			continue;
		}

		pRenderTargetContainer->AdvanceUpdateInterval(m_DeltaTime);
		if (pRenderTargetContainer->IsReadyForRender())
		{
			pRenderTargetContainer->ResetReadyForRender();
			windowsFlaggedForRender.push_back(pWindow);
			renderTargetsFlaggedForRender.push_back(pRenderTargetContainer);
		}
		else
		{
			continue; //Do not render render targets if they are not ready to be rendered. This only happens when limiting refresh rate at render target level.
		}
	}

	for (auto& offscreenRenderTargets : m_OffScreenRenderTargetContainers)
	{
		if (!offscreenRenderTargets->IsActive())
		{
			continue;
		}

		offscreenRenderTargets->AdvanceUpdateInterval(m_DeltaTime);
		if (offscreenRenderTargets->IsReadyForRender())
		{
			offscreenRenderTargets->ResetReadyForRender();
			renderTargetsFlaggedForRender.push_back(offscreenRenderTargets);
		}
		else
		{
			continue; //Do not render render targets if they are not ready to be rendered. This only happens when limiting refresh rate at render target level.
		}
	}

	for (auto rt : renderTargetsFlaggedForRender) //Render each of our scenes to our render targets where applicable
	{
		m_Renderer.ClearRenderTarget(rt);
		m_Renderer.RenderSceneInRenderTargetContainer(rt); //If no camera is bound with an attached scene, this will just exit
	}

	OnPreRenderULViews(); //for if we want to draw anything before the UL views have been rendered

	for (auto pWindow : windowsFlaggedForRender) //Render each of our ultralight views to each window
	{
		WeakWrapper<RenderTargetContainer> pRenderTargetContainer = pWindow->GetRenderTargetContainer();
		m_Renderer.PrepareFor2DRendering(pRenderTargetContainer);
		for (WeakWrapper<UltralightView> pUltralightView : pWindow->GetSortedUltralightViews())
		{
			m_Renderer.RenderUltralightView(pUltralightView.get());
		}
	}

	OnPostRenderULViews(); //for if we want to draw anything after the UL views have been rendered

	for (auto rt : renderTargetsFlaggedForRender)
	{
		rt->ResolveIfNecessary();
	}

	//Present each window's render target
	for (auto pWindow : windowsFlaggedForRender)
	{
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
