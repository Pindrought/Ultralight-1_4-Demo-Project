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
	DebugBreak();
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
	}
	if (parms.Style & WindowStyle::NoBorder)
	{
		UINT styleFix = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX; //For NoBorder we must exclude these
		styleFix = ~styleFix; //bit flip
		styleWinAPI &= styleFix; //and with flipped bit to remove those unwanted flags
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
		extendedWindowsStyle |= WS_EX_LAYERED;
	}
	else
	{
		m_TransparencyAllowed = false;
	}

	if (parms.Style & WindowStyle::Topmost)
	{
		extendedWindowsStyle |= WS_EX_TOPMOST;
	}

	if (m_HWND != NULL)
	{
		DestroyWindow(m_HWND);
		UnregisterClassA(m_WindowClassName.c_str(), GetModuleHandle(NULL));
		m_HWND = NULL;
	}

	RegisterWindowClass();

	//Create Window
	m_HWND = CreateWindowExA(extendedWindowsStyle, //Extended Windows style. For other options, see: https://msdn.microsoft.com/en-us/library/windows/desktop/ff700543(v=vs.85).aspx
							 m_WindowClassName.c_str(), //Window class name
							 m_Title.c_str(), //Window Title
							 styleWinAPI, //Windows style - See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms632600(v=vs.85).aspx
							 wr.left, //Window X Position
							 wr.top, //Window Y Position
							 wr.right - wr.left, //Window Width
							 wr.bottom - wr.top, //Window Height
							 NULL, //Handle to parent of this window. Since this is the first window, it has no parent window.
							 NULL, //Handle to menu or child window identifier. Can be set to NULL and use menu in WindowClassEx if a menu is desired to be used.
							 GetModuleHandle(NULL), //Handle to the instance of module to be used with this window
							 this); //Parameter passed to create window 'WM_NCCREATE'

	if (m_HWND == NULL)
	{
		DWORD error = GetLastError();
		return false;
	}
	//Show/focus Window
	ShowWindow(m_HWND, SW_SHOW);
	SetForegroundWindow(m_HWND);
	SetFocus(m_HWND);

	if (m_ClickThroughEnabled)
	{
		if (ToggleClickthrough(m_ClickThroughEnabled) == false)
			return false;
	}

	if (m_TransparencyUsed)
	{
		if (SetWindowAlpha(parms.WindowAlpha) == false)
			return false;
	}

	if (m_ColorRefUsed)
	{
		SetWindowColorKey(m_ColorRef);
	}

	if (!InitializeSwapchain())
	{
		return false;
	}

	if (!InitializeRenderTargetContainer())
	{
		return false;
	}

	UltralightManager* pUltralightManager = UltralightManager::GetInstance();
	assert(pUltralightManager != nullptr);

	return true;
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

BYTE Window::GetAlpha() const
{
	return m_Alpha;
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

bool Window::SetWindowAlpha(float alphaFloat)
{
	alphaFloat = std::max(0.0f, alphaFloat);
	alphaFloat = std::min(1.0f, alphaFloat);
	BYTE alphaByte = 255 * alphaFloat;
	m_Alpha = alphaByte;
	m_TransparencyUsed = true;
	if (m_ColorRefUsed)
	{
		return SetLayeredWindowAttributes(m_HWND, m_ColorRef, alphaByte, LWA_ALPHA | LWA_COLORKEY);
	}
	else
	{
		return SetLayeredWindowAttributes(m_HWND, NULL, alphaByte, LWA_ALPHA);
	}
}

bool Window::SetWindowColorKey(COLORREF colorKey)
{
	m_ColorRef = colorKey;
	m_ColorRefUsed = true;
	if (m_TransparencyUsed)
	{
		return SetLayeredWindowAttributes(m_HWND, m_ColorRef, m_Alpha, LWA_ALPHA | LWA_COLORKEY);
	}
	else
	{
		return SetLayeredWindowAttributes(m_HWND, m_ColorRef, 100,  LWA_COLORKEY);
	}
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
		if (GetId() == 0) //Primary window being closed?
		{
			pEngine->SetRunning(false);
		}
		pEngine->OnWindowDestroyCallback(m_Id);

		DestroyWindow(hwnd);
		s_WindowsIDManager.StoreId(GetId());
		pEngine->CleanupWindow(m_Id);
		UltralightManager* pUltralightManager = UltralightManager::GetInstance();
		assert(pUltralightManager != nullptr);
		pUltralightManager->RemoveWindowId(m_Id);
		return 0;
	}
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
			if (!ResizeSwapChainAndRenderTargetContainer())
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

bool Window::InitializeSwapchain()
{
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
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (pD3D->IsTearingSupported())
	{
		swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // for vsync off or this
	}

	hr = dxgiFactory->CreateSwapChainForHwnd(pDevice.Get(),
											 m_HWND,
											 &swapChainDesc,
											 NULL,
											 NULL,
											 &m_SwapChain);

	ReturnFalseIfFail(hr, "Failed to create D3D11 SwapChain for window.");

#ifdef _DEBUG
	std::string name = "WindowSwapchain_" + std::to_string(m_Id);
	hr = m_SwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
#endif

	return true;
}

bool Window::InitializeRenderTargetContainer()
{
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

void Window::RegisterWindowClass()
{
	WNDCLASSEXA wc = {}; //Our Window Class (This has to be filled before our window can be created) See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Flags [Redraw on width/height change from resize/movement] See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
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
	RegisterClassExA(&wc); // Register the class so that it is usable.
}

bool Window::ResizeSwapChainAndRenderTargetContainer()
{
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
