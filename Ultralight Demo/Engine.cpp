#include <PCH.h>
#include "Engine.h"
#include "Graphics/Renderer.h"
#include "Window/Window.h"
#include "Window/InputController/InputController.h"
#include "Misc/CursorManager.h"

Engine* Engine::s_Instance = nullptr;

Engine* Engine::GetInstance()
{
	return s_Instance;
}

InputController* Engine::GetInputController()
{
	return &m_InputController;
}

bool Engine::Initialize(WindowCreationParameters windowParms)
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

	if (!m_UltralightMgr.Initialize())
	{
		FatalError("Failed to initialize Ultralight. Program must now abort.");
		return false;
	}

	for(int i=0; i<2; i++)
	{
		windowParms.Title = "GPU Renderer" + std::to_string(i);
		windowParms.Style = windowParms.Style;
		shared_ptr<Window> window = SpawnWindow(windowParms);
		if (i == 0)
		{
			m_PrimaryWindow = window;
			if (m_PrimaryWindow == nullptr)
			{
				FatalError("Failed to initialize primary window. Program must now abort.");
				return false;
			}
		}

		UltralightViewCreationParameters parms;
		parms.Width = windowParms.Width;
		parms.Height = windowParms.Height;
		parms.IsAccelerated = true;
		//parms.ForceMatchWindowDimensions = true;
		parms.IsTransparent = true;

		shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(parms);
		pView->LoadURL("file:///Samples/BorderlessWindow/BorderlessWindow.html");
		m_UltralightMgr.SetViewToWindow(pView->GetId(), window->GetId());

		{
			UltralightViewCreationParameters parms;
			parms.Width = windowParms.Width;
			parms.Height = windowParms.Height - 24;
			parms.IsAccelerated = true;
			//parms.ForceMatchWindowDimensions = true;
			parms.IsTransparent = true;
			parms.Position.y = 24;

			shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(parms);
			pView->LoadURL("http://www.google.com");
			m_UltralightMgr.SetViewToWindow(pView->GetId(), window->GetId());
		}
	}

	for (int i = 0; i < 1; i++)
	{
		windowParms.Title = "CPU Renderer" + std::to_string(i);
		windowParms.Style = windowParms.Style;
		shared_ptr<Window> window = SpawnWindow(windowParms);

		UltralightViewCreationParameters parms;
		parms.Width = windowParms.Width;
		parms.Height = windowParms.Height;
		parms.IsAccelerated = false;
		//parms.ForceMatchWindowDimensions = true;
		parms.IsTransparent = true;

		shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(parms);
		pView->LoadURL("file:///Samples/BorderlessWindow/BorderlessWindow.html");
		m_UltralightMgr.SetViewToWindow(pView->GetId(), window->GetId());
	}

	//{
	//	windowParms.Title = "GPU Renderer";
	//	std::shared_ptr<Window> window = SpawnWindow(windowParms);
	//	if (window == nullptr)
	//	{
	//		FatalError("Failed to initialize window. Program must now abort.");
	//		return false;
	//	}

	//	UltralightViewCreationParameters parms;
	//	parms.Width = windowParms.Width;
	//	parms.Height = windowParms.Height;
	//	parms.IsAccelerated = true;
	//	parms.ForceMatchWindowDimensions = true;

	//	shared_ptr<UltralightView> gpuView = m_UltralightMgr.CreateUltralightView(parms);

	//	gpuView->LoadURL("file:///TestFunction.html");
	//	m_UltralightMgr.SetViewToWindow(gpuView->GetId(), window->GetId());
	//}

	SetRunning(true);

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

void Engine::Tick()
{
	//Process Input Events
	auto& keyboard = m_InputController.m_Keyboard;
	auto& mouse = m_InputController.m_Mouse;
	while (mouse.EventBufferIsEmpty() == false)
	{
		MouseEvent mouseEvent = mouse.ReadEvent();
		bool dispatchedToHtml = m_UltralightMgr.FireMouseEvent(&mouseEvent);
		if (m_WindowDragInfo.DragInProgress)
		{
			if (mouseEvent.GetType() == MouseEvent::Type::MouseUp)
			{
				m_WindowDragInfo.DragInProgress = false;
				m_WindowDragInfo.pWindowBeingDragged = nullptr;
			}
			if (mouseEvent.GetType() == MouseEvent::Type::MouseMoveRaw)
			{
				HWND hwnd = m_WindowDragInfo.pWindowBeingDragged->GetHWND();
				POINT p;
				if (GetCursorPos(&p))
				{
					int deltaX = p.x - m_WindowDragInfo.DragStartMousePosition.x;
					int deltaY = p.y - m_WindowDragInfo.DragStartMousePosition.y;

					SetWindowPos(hwnd, NULL,
								 m_WindowDragInfo.DragStartWindowPosition.x + deltaX,
								 m_WindowDragInfo.DragStartWindowPosition.y + deltaY, 0, 0, SWP_NOSIZE);
				}
			}
		}
		if (dispatchedToHtml == false) //Because of the way the window is being initialized (without a default cursor), it is
		{							   //possible to have the cursor state changed ex. resize border on window and have it not be
									   //changed back to normal if not hovering over an Ultralight View to reset it
			CursorType cursor = CursorManager::GetCursor();
			if (mouseEvent.GetType() == MouseEvent::Type::MouseMove)
			{
				//TODO: Maybe add error checking?
				shared_ptr<Window> pWindow = m_WindowIdToWindowInstanceMap[mouseEvent.GetWindowId()];
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
		bool dispatchedToHtml = m_UltralightMgr.FireScrollEvent(&scrollEvent);
	}
	while (keyboard.EventBufferIsEmpty() == false)
	{
		KeyboardEvent keyboardEvent = keyboard.ReadEvent();
		bool dispatchedtoHtml = m_UltralightMgr.FireKeyboardEvent(&keyboardEvent);
		if (keyboardEvent.GetKeyCode() == 'A'
			/*keyboardEvent.GetType() == KeyboardEvent::Type::KeyDown &&
			keyboardEvent.IsAutoRepeat() == false*/)
		{
			string x = "hey";
			x += "zz";
			vector<EZJSParm> parms = {
				x,
				"Hello Parm 2",
				{ "Hello", "World", 5, false},
				{
					kvp("George", x),
					kvp("Bill", 16),
					kvp("Susan", "Unknown")
				}
			};

			std::string exception;
			EZJSParm returnValue;
			if (!m_UltralightTestView->CallJSFnc("TestFunction", 
													parms, //Parms to pass to stack for function call
													returnValue, //output variable to hold return value if js fnc returns anything
													exception))
			{
				//exception occured and stored in exception
			}
		}
	}

	RenderFrame();
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
	m_UltralightMgr.RegisterWindow(window);

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
	return nullptr;
}

EZJSParm Engine::OnEventCallbackFromUltralight(int32_t viewId, 
										   string eventName, 
										   vector<EZJSParm> parameters)
{
	if (eventName == "RequestTitle")
	{
		auto pView = m_UltralightMgr.GetViewFromId(viewId);
		int32_t windowId = pView->GetWindowId();
		auto pWindow = m_WindowIdToWindowInstanceMap[windowId];
		return pWindow->GetTitle();
	}

	if (eventName == "CloseWindow")
	{
		auto pView = m_UltralightMgr.GetViewFromId(viewId);
		int32_t windowId = pView->GetWindowId();
		auto pWindow = m_WindowIdToWindowInstanceMap[windowId];
		HWND hwnd = pWindow->GetHWND();
		SendMessage(hwnd, WM_CLOSE, NULL, NULL);
		return EZJSParm();
	}

	if (eventName == "BeginWindowDrag")
	{
		if (GetCursorPos(&m_WindowDragInfo.DragStartMousePosition))
		{
			//TODO: Add error checking
			auto pView = m_UltralightMgr.GetViewFromId(viewId);
			int32_t windowId = pView->GetWindowId();
			auto pWindow = m_WindowIdToWindowInstanceMap[windowId];
			HWND hwnd = pWindow->GetHWND();
			RECT rect = { NULL };
			if (GetWindowRect(hwnd, &rect))
			{
				m_WindowDragInfo.DragStartWindowPosition.x = rect.left;
				m_WindowDragInfo.DragStartWindowPosition.y = rect.top;

				m_WindowDragInfo.DragInProgress = true;
				m_WindowDragInfo.pWindowBeingDragged = pWindow;
			}
		}
		return EZJSParm();
	}

	return EZJSParm();
}

void Engine::OnWindowDestroyCallback(int32_t windowId)
{
	//Note: The OnWindowDestroyCallback function is called right before the actual window is destroyed.
	//TODO: Add error checking
	shared_ptr<Window> pWindow = m_WindowIdToWindowInstanceMap[windowId];
}

void Engine::RenderFrame()
{
	m_UltralightMgr.UpdateViews();

	float deltaTime = 0;
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
		}
		else
		{
			continue; //Do not render render targets if they are not ready to be rendered. This only happens when limiting refresh rate at render target level.
		}

		m_Renderer.ClearRenderTarget(pRenderTargetContainer);

		ID3D11DeviceContext* pContext = m_Renderer.GetD3D()->m_Context.Get();

		for (shared_ptr<UltralightView> pUltralightView : pWindow->GetSortedUltralightViews())
		{
			m_Renderer.RenderUltralightView(pUltralightView.get());
		}

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

	m_UltralightMgr.RefreshViewDisplaysForAnimations();
}
