#include <PCH.h>
#include "Window.h"
#include "../Engine.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/D3DClass.h"
#include "InputController/InputController.h"
#include "../Misc/IDPoolManager.h"
#include "../UltralightImpl/UltralightManager.h"

static IDPoolManager<int32_t> s_WindowsIDManager;

Window::~Window()
{
	BOOL result = UnregisterClassA(m_WindowClassName.c_str(), GetModuleHandle(NULL));
}

bool Window::Initialize(const WindowCreationParameters& parms)
{
	if (m_Id == -1)
	{
		m_Id = GetAvailableWindowId();
	}

	m_Width = parms.Width;
	m_Height = parms.Height;
	m_Title = parms.Title;
	m_WindowClassName += std::to_string(m_Id);
	m_Style = parms.Style;
	m_DirectCompositionEnabled = parms.Style & WindowStyle::TransparencyAllowed;

	static bool raw_input_initialized = false;
	if (raw_input_initialized == false) //This just has to be done once for raw mouse move events.
	{
		RAWINPUTDEVICE rid;

		rid.usUsagePage = 0x01; //Mouse
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = NULL;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			return false;
		}

		raw_input_initialized = true;
		LOGINFO("RAW INPUT INITIALIZED.");
	}

	//Determine window x/y position
	int xPos = parms.XPosition;
	if (parms.XPosition == INT_MAX) //if no xpos entered, set to center of screen
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int halfScreenWidth = screenWidth / 2;
		int halfWindowWidth = parms.Width / 2;
		int centerScreenX = halfScreenWidth - halfWindowWidth;
		xPos = centerScreenX;
	}

	int yPos = parms.YPosition;
	if (yPos == INT_MAX) //if no ypos entered, set to center of screen
	{
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int halfScreenHeight = screenHeight / 2;
		int halfWindowHeight = parms.Height / 2;
		int centerScreenY = halfScreenHeight - halfWindowHeight;
		yPos = centerScreenY;
	}
	//Determine Window Rect
	RECT wr; //Window Rectangle
	wr.left = xPos;
	wr.top = yPos;
	wr.right = wr.left + parms.Width;
	wr.bottom = wr.top + parms.Height;
	RECT tempwr = wr;

	DWORD styleWinAPI = 0;
	styleWinAPI |= WS_POPUP;

	if (parms.Style & WindowStyle::ExitButton)
	{
		styleWinAPI |= WS_SYSMENU | WS_CAPTION;
	}
	if (parms.Style & WindowStyle::MinimizeAvailable)
	{
		styleWinAPI |= WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	}
	if (parms.Style & WindowStyle::MaximizeAvailable)
	{
		styleWinAPI |= WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX;
	}
	if (parms.Style & WindowStyle::Resizable)
	{
		styleWinAPI |= WS_SIZEBOX;
		if (parms.Style & WindowStyle::NoBorder) //See WindowHelperForBorderlessResizable class... When combining the two, it's a lot of extra work.
		{
			styleWinAPI |= WS_OVERLAPPED;
			m_IsBorderlessResizable = true;
		}
	}
	else
	{
		if (parms.Style & WindowStyle::NoBorder)
		{
			UINT styleFix = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX; //For NoBorder we must exclude these
			styleFix = ~styleFix; //bit flip
			styleWinAPI &= styleFix; //and with flipped bit to remove those unwanted flags
		}
	}

	BOOL result = AdjustWindowRect(&wr, styleWinAPI, FALSE);
	if (result == 0) //If adjustwindowrect fails...
	{
		CriticalError("AdjustWindowRect failed. That is not good.");
		return false;
	}

	//Register Window Class
	DWORD extendedWindowsStyle = 0;
	if (parms.Style & WindowStyle::TransparencyAllowed)
	{
		extendedWindowsStyle |= WS_EX_NOREDIRECTIONBITMAP;
	}

	if (parms.Style & WindowStyle::Topmost)
	{
		extendedWindowsStyle |= WS_EX_TOPMOST;
	}

	if (m_HWND != NULL)
	{
		LOGINFO("WINDOW EXISTING HANDLE CLEANED UP.");
		DestroyWindow(m_HWND);
		BOOL result = UnregisterClassA(m_WindowClassName.c_str(), GetModuleHandle(NULL));
		m_HWND = NULL;
	}

	if (!RegisterWindowClass())
	{
		return false;
	}


	//Create Window
	m_HWND = CreateWindowExA(extendedWindowsStyle, //Extended Windows style. For other options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
							 m_WindowClassName.c_str(), //Window class name
							 m_Title.c_str(), //Window Title
							 styleWinAPI, //Windows style - See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
							 wr.left, //Window X Position
							 wr.top, //Window Y Position
							 wr.right - wr.left, //Window Width
							 wr.bottom - wr.top, //Window Height
							 parms.ParentWindow, //Handle to parent of this window.
							 NULL, //Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a menu is desired to be used.
							 GetModuleHandle(NULL), //Handle to the instance of module to be used with this window
							 this); //Parameter passed to create window 'WM_NCCREATE'

	if (m_HWND == NULL)
	{
		DWORD error = GetLastError();
		return false;
	}

	if (m_IsBorderlessResizable)
	{
		m_BRWData.Width = m_Width;
		m_BRWData.Height = m_Height;
		m_BRWData.Hwnd = m_HWND;
		WindowHelperForBorderlessResizable::HandleCompositionChanged(this);
		WindowHelperForBorderlessResizable::HandleThemeChanged(this);
	}

	//Show/focus Window
	ShowWindow(m_HWND, SW_SHOW);
	SetForegroundWindow(m_HWND);
	SetFocus(m_HWND);

	/*if (m_ClickThroughEnabled)
	{
		if (ToggleClickthrough(m_ClickThroughEnabled) == false)
			return false;
	}*/
	
	if (!InitializeSwapchain())
	{
		LOGINFO("FAILED TO INITIALIZE WINDOW SWAPCHAIN.");
		return false;
	}

	if (!InitializeRenderTargetContainer())
	{
		LOGINFO("FAILED TO INITIALIZE WINDOW RENDER TARGET CONTAINER.");
		return false;
	}

	LOGINFO("INITIALIZED WINDOW RENDER TARGET CONTAINER.");

	UltralightManager* pUltralightManager = UltralightManager::GetInstance();
	assert(pUltralightManager != nullptr);

	return true;
}

void Window::Enable()
{
	m_IsEnabled = true;
	EnableWindow(m_HWND, TRUE);
}

void Window::Disable()
{
	m_IsEnabled = false;
	EnableWindow(m_HWND, FALSE);
}

int32_t Window::GetId() const
{
	return m_Id;
}

HWND Window::GetHWND() const
{
	return m_HWND;
}

int32_t Window::GetWidth() const
{
	return m_Width;
}

int32_t Window::GetHeight() const
{
	return m_Height;
}

string Window::GetTitle() const
{
	return m_Title;
}

WindowStyle Window::GetStyle() const
{
	return m_Style;
}

bool Window::ToggleClickthrough(bool clickthrough)
{
	LONG_PTR currentStyle = GetWindowLongPtr(m_HWND, GWL_EXSTYLE);
	if (clickthrough) //Enable click through
	{
		//currentStyle |= WS_EX_TRANSPARENT;
		currentStyle |= (WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);
	}
	else //Disable click through
	{
		LONG_PTR removeStyle = ~WS_EX_TRANSPARENT;
		currentStyle &= removeStyle;
	}
	if (SetWindowLongPtr(m_HWND, GWL_EXSTYLE, currentStyle) == 0)
		return false;
	return true;
}

void Window::StartDrag()
{
	ReleaseCapture();
	SendMessageW(m_HWND, WM_NCLBUTTONDOWN, HTCAPTION, 0);
}

void Window::StopDrag()
{
	SendMessageW(m_HWND, WM_NCLBUTTONUP, HTCAPTION, 0);
}

IDXGISwapChain1* Window::GetSwapChainPtr()
{
	return m_SwapChain.Get();
}

RenderTargetContainer* Window::GetRenderTargetContainer()
{
	assert(m_RenderTargetContainer != nullptr);
	return m_RenderTargetContainer.get();
}

LRESULT Window::WindowProcA(HWND hwnd,
							UINT uMsg,
							WPARAM wParam,
							LPARAM lParam)
{
	//LOGINFO(strfmt("%d -- %d", (int)hwnd, uMsg).c_str());
	Engine* pEngine = Engine::GetInstance();
	assert(pEngine != nullptr);

	InputController* pInputController = pEngine->GetInputController();

	auto& mouse = pInputController->m_Mouse;
	auto& keyboard = pInputController->m_Keyboard;
	switch (uMsg)
	{
	case WM_PAINT: //This is kind of a hacky way to prevent getting stuck in modal loop while resizing/dragging window
		if (m_ResizeOrMoveInProgress)
		{
			pEngine->Tick();
			pEngine->RenderFrame();
		}
		else
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}

		return 0;
	case WM_CLOSE: //Window closed
	{
		auto pEngine = Engine::GetInstance();
		pEngine->OnWindowDestroyStartCallback(m_Id);

		pEngine->GetInputController()->ClearEventsForWindow(m_Id);

		DestroyWindow(hwnd);
		s_WindowsIDManager.StoreId(GetId());
		pEngine->CleanupWindow(m_Id);
		UltralightManager* pUltralightManager = UltralightManager::GetInstance();
		assert(pUltralightManager != nullptr);
		int id = m_Id; //Need to store the window, because it will be deallocated after RemoveWindowId is called
		pUltralightManager->RemoveWindowId(id);
		pEngine->OnWindowDestroyEndCallback(id);
		return 0;
	}
	case WM_DWMCOMPOSITIONCHANGED:
		if (m_IsBorderlessResizable)
		{
			WindowHelperForBorderlessResizable::HandleCompositionChanged(this);
			return 0;
		}
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	case WM_NCACTIVATE:
		if (m_IsBorderlessResizable)
		{
			/* DefWindowProc won't repaint the window border if lParam (normally a
		   HRGN) is -1. This is recommended in:
		   https://blogs.msdn.microsoft.com/wpfsdk/2008/09/08/custom-window-chrome-in-wpf/ */
			return DefWindowProcA(hwnd, uMsg, wParam, -1);
		}
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	case WM_NCCALCSIZE:
		if (m_IsBorderlessResizable)
		{
			WindowHelperForBorderlessResizable::HandleNCCalcSize(this, wParam, lParam);
			return 0;
		}
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	case WM_NCHITTEST:
		if (m_IsBorderlessResizable)
		{
			return WindowHelperForBorderlessResizable::HandleNCHitTest(this, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	case WM_NCPAINT:
		/* Only block WM_NCPAINT when composition is disabled. If it's blocked
		   when composition is enabled, the window shadow won't be drawn. */
		if (m_IsBorderlessResizable)
		{
			if (m_BRWData.CompositionEnabled == false)
				return 0;
		}
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);

	case WM_NCUAHDRAWCAPTION:
	case WM_NCUAHDRAWFRAME:
		if (m_IsBorderlessResizable)
			return 0;
		/* These undocumented messages are sent to draw themed window borders.
		   Block them to prevent drawing borders over the client area. */
		return 0;




	//Mouse Messages
	case WM_MOUSEMOVE:
		mouse.OnWindowsMouseMessage(m_Id, uMsg, wParam, lParam);
		return 0;
	case WM_MOUSEWHEEL:
		mouse.OnWindowsScrollMessage(m_Id, uMsg, wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		mouse.OnWindowsMouseMessage(m_Id, uMsg, wParam, lParam);
		SetCapture(hwnd);
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		mouse.OnWindowsMouseMessage(m_Id, uMsg, wParam, lParam);
		ReleaseCapture();
		return 0;
		//Keyboard Messages
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
		keyboard.OnWindowsKeyboardMessage(m_Id, uMsg, wParam, lParam);
		return 0;
	case WM_INPUT:
	{
		UINT dataSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
						RID_INPUT,
						NULL,
						&dataSize,
						sizeof(RAWINPUTHEADER)); //Need to populate data size first

		if (dataSize > 0)
		{
			std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					mouse.OnMouseMoveRaw(m_Id, raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}

		return DefWindowProcA(hwnd, uMsg, wParam, lParam); //Need to call DefWindowProc for WM_INPUT messages
	}
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	{
		if (uMsg == WM_ENTERSIZEMOVE)
		{
			m_ResizeOrMoveInProgress = true;
			InvalidateRect(hwnd, nullptr, FALSE);
		}
		else
		{
			m_ResizeOrMoveInProgress = false;
			if (!ResizeSwapChainAndRenderTargetContainer())
			{
				CriticalError("Failed to resize swapchain and render target container."); //Not really sure what to do if this happens
			}
		}

		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 32; //Minimum Width
		lpMMI->ptMinTrackSize.y = 32; //Minimum Height
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	case WM_SIZE:
	{
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);
		if (width >= 32 && height >= 32 &&
			width != m_Width ||
			height != m_Height)
		{
			m_Width = LOWORD(lParam);
			m_Height = HIWORD(lParam);
			if (ResizeSwapChainAndRenderTargetContainer())
			{
				pEngine->OnWindowResizeCallback(this);
			}
			else
			{
				CriticalError("Failed to resize swapchain and render target container on WM_SIZE."); //Not really sure what to do if this happens
			}
		}

		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	case WM_SIZING:
	{
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
}

const list<shared_ptr<UltralightView>>& Window::GetSortedUltralightViews()
{
	return m_UltralightViewsSorted;
}

bool Window::IsWindowMaximized() const
{
	return m_IsMaximized;
}

void Window::SetPosition(int x, int y, int width, int height)
{
	UINT flags = 0;
	if (width == -1 || height == -1)
	{
		flags = SWP_NOSIZE;
	}
	SetWindowPos(m_HWND, NULL, x, y, m_Width, m_Height, flags);
}

void Window::Maximize()
{
	m_IsMaximized = true;
	ShowWindow(m_HWND, SW_SHOWMAXIMIZED);
}

void Window::Restore()
{
	m_IsMaximized = false;
	ShowWindow(m_HWND, SW_RESTORE);
}

void Window::Show()
{
	ShowWindow(m_HWND, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow(m_HWND, SW_HIDE);
}

void Window::Close()
{
	if (m_CloseInitiated == true)
	{
		return;
	}
	m_CloseInitiated = true;
	SendMessageA(m_HWND, WM_CLOSE, NULL, NULL);
}

bool Window::InitializeSwapchain()
{
	LOGINFO("INITIALIZING WINDOW SWAPCHAIN.");

	D3DClass* pD3D = D3DClass::GetInstance();
	ComPtr<ID3D11Device> pDevice = pD3D->m_Device.Get();

	HRESULT hr = S_OK;
	// First, retrieve the underlying DXGI Device from the D3D Device
	ComPtr<IDXGIDevice2> dxgiDevice;
	hr = pDevice.As(&dxgiDevice);
	ReturnFalseIfFail(hr, "Failed to retrieve underlying DXGI Device from D3D11 Device.");

	// Identify the physical adapter (GPU or card) this device is running on.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
	ReturnFalseIfFail(hr, "Failed to retrieve IDXGI Adapter from DXGI Device.");

	// And obtain the factory object that created it.
	ComPtr<IDXGIFactory2> dxgiFactory;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
	ReturnFalseIfFail(hr, "Failed to retrieve IDXGI Factory from DXGI Adapter.");

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = m_Width;
	swapChainDesc.Height = m_Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (pD3D->IsTearingSupported())
	{
		swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // for vsync off or this
	}

	if (m_DirectCompositionEnabled)
	{
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; 
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		hr = dxgiFactory->CreateSwapChainForComposition(pDevice.Get(),
														&swapChainDesc,
														nullptr,
														&m_SwapChain);
		ReturnFalseIfFail(hr, "Failed to create D3D11 SwapChain for composition.");
#ifdef _DEBUG
		std::string name = "CompositionSwapchain_" + std::to_string(m_Id);
		hr = m_SwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
#endif
		auto pCompDevice = pD3D->m_CompositionDevice.Get();

		hr = pCompDevice->CreateTargetForHwnd(m_HWND, TRUE, &m_DirectCompositionTarget);
		ReturnFalseIfFail(hr, "Failed to create direct composition target for window.");

		hr = pCompDevice->CreateVisual(&m_DirectCompositionVisual);
		ReturnFalseIfFail(hr, "Failed to create direct composition visual for window.");

		hr = m_DirectCompositionVisual->SetContent(m_SwapChain.Get());
		ReturnFalseIfFail(hr, "Failed to set direct composition visual for window swapchain.");

		hr = m_DirectCompositionTarget->SetRoot(m_DirectCompositionVisual.Get());
		ReturnFalseIfFail(hr, "Failed to set direct composition target root to visual.");

		hr = pCompDevice->Commit();
		ReturnFalseIfFail(hr, "Failed to commit direct composition commands.");

	}
	else
	{
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		hr = dxgiFactory->CreateSwapChainForHwnd(pDevice.Get(),
												 m_HWND,
												 &swapChainDesc,
												 nullptr,
												 nullptr,
												 &m_SwapChain);
		ReturnFalseIfFail(hr, "Failed to create D3D11 SwapChain for window.");
#ifdef _DEBUG
		std::string name = "WindowSwapchain_" + std::to_string(m_Id);
		hr = m_SwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
#endif
	}
	
	LOGINFO("WINDOW SWAPCHAIN SUCCESSFULLY INITIALIZED.");

	return true;
}

bool Window::InitializeRenderTargetContainer()
{
	LOGINFO("INITIALIZING WINDOW RENDER TARGET CONTAINER.");
	m_RenderTargetContainer = std::make_shared<RenderTargetContainer>();
	return m_RenderTargetContainer->InitializeWindowRenderTarget(this);
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd,
								   UINT uMsg,
								   WPARAM wParam,
								   LPARAM lParam)
{
	switch (uMsg)
	{
		/*case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;*/

	default:
	{
		// retrieve ptr to window class
		Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		// forward message to window class handler
		return pWindow->WindowProcA(hwnd, uMsg, wParam, lParam);
	}
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		if (pWindow == nullptr) //Sanity check
		{
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProcA(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
}

bool Window::RegisterWindowClass()
{
	WNDCLASSEXA wc = {}; //Our Window Class (This has to be filled before our window can be created) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
	if (m_DirectCompositionEnabled)
	{
		wc.style = CS_HREDRAW | CS_VREDRAW; //Flags [Redraw on width/height change from resize/movement] See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	}
	else
	{
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Flags [Redraw on width/height change from resize/movement] See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	}
	wc.lpfnWndProc = HandleMessageSetup; //Pointer to Window Proc function for handling messages from this window
	wc.cbClsExtra = 0; //# of extra bytes to allocate following the window-class structure. We are not currently using this.
	wc.cbWndExtra = 0; //# of extra bytes to allocate following the window instance. We are not currently using this.
	wc.hInstance = GetModuleHandle(NULL); //Handle to the instance that contains the Window Procedure
	wc.hIcon = NULL;   //Handle to the class icon. Must be a handle to an icon resource. We are not currently assigning an icon, so this is null.
	wc.hIconSm = NULL; //Handle to small icon for this class. We are not currently assigning an icon, so this is null.
	wc.hCursor = NULL; //Default Cursor - If we leave this null, we have to explicitly set the cursor's shape each time it enters the window.
	wc.hbrBackground = NULL; //Handle to the class background brush for the window's background color - we will leave this blank for now and later set this to black. For stock brushes, see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd144925(v=vs.85).aspx
	wc.lpszMenuName = NULL; //Pointer to a null terminated character string for the menu. We are not using a menu yet, so this will be NULL.
	wc.lpszClassName = m_WindowClassName.c_str(); //Pointer to null terminated string of our class name for this window.
	wc.cbSize = sizeof(WNDCLASSEXA); //Need to fill in the size of our struct for cbSize
	ATOM result = RegisterClassExA(&wc); // Register the class so that it is usable.
	if (result == 0)
	{
		return false;
	}
	LOGINFO("REGISTERED WINDOW CLASS.");
	return true;
}

bool Window::ResizeSwapChainAndRenderTargetContainer()
{
	if (m_RenderTargetContainer == nullptr)
		return true;

	m_RenderTargetContainer->ResetRenderTargetsForResize();

	DXGI_SWAP_CHAIN_DESC1 desc;
	HRESULT hr = m_SwapChain->GetDesc1(&desc);
	ReturnFalseIfFail(hr, "Failed to get swapchain desc for resize.");

	hr = m_SwapChain->ResizeBuffers(2,
									m_Width,
									m_Height,
									desc.Format,
									desc.Flags);
	ReturnFalseIfFail(hr, "Failed to resize window swapchain.");

	m_RenderTargetContainer->Resize(m_Width, m_Height);

	for (auto& view : m_UltralightViewsSorted)
	{
		if (view->ShouldMatchWindowDimensions())
		{
			view->Resize(m_Width, m_Height);
		}
	}
}

int32_t Window::GetAvailableWindowId()
{

	//Technically it is possible for this to overflow if you create 2,147,486,647 windows while the application is open.
	//You could create a reserve pool to store available id's once a window gets destroyed to reuse them and replace this function with that.
	int32_t id = s_WindowsIDManager.GetNextId();
	return id;
}
