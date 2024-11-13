#include "PCH.h"
#include "DemoScreenShareHelper.h"
#include "../Misc/CursorManager.h"
#include "../Graphics/PipelineState.h"

HHOOK DemoScreenShareHelper::s_HandleMouseHook = NULL;

bool DemoScreenShareHelper::SetHook()
{
	RemoveHook();
	s_HandleMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, nullptr, 0);
	if (s_HandleMouseHook == NULL) 
	{
		return false;
	}
	return true;
}

void DemoScreenShareHelper::RemoveHook()
{
	if (s_HandleMouseHook != NULL) 
	{
		UnhookWindowsHookEx(s_HandleMouseHook);
		s_HandleMouseHook = NULL;
	}
}

LRESULT CALLBACK DemoScreenShareHelper::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) 
	{
		auto* pMouseStruct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
		DemoScreenShareHelper* pEngine = (DemoScreenShareHelper*)Engine::GetInstance();

		switch (wParam) {
		case WM_MOUSEMOVE:
			if (pEngine->OnMouseMove(pMouseStruct->pt.x, pMouseStruct->pt.y))
			{
				//return 1;
			}
			break;

		case WM_LBUTTONDOWN:
			if (pEngine->OnMouseDown(pMouseStruct->pt.x, pMouseStruct->pt.y))
			{
				return 1;
			}
			break;

		case WM_LBUTTONUP:
			if (pEngine->OnMouseUp(pMouseStruct->pt.x, pMouseStruct->pt.y))
			{
				return 1;
			}
			break;


		default:
			break;
		}
	}
	return CallNextHookEx(s_HandleMouseHook, nCode, wParam, lParam); // Pass other events to the next hook
}

bool DemoScreenShareHelper::Startup()
{
	if (!m_ScreenCapper.Initialize())
	{
		return false;
	}

	WindowCreationParameters windowParms;
	windowParms.Width = 200;
	windowParms.Height = 125;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Screen Share Helper";
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

	m_View = m_UltralightMgr->CreateUltralightView(parms);
	if (m_View.expired())
	{
		FatalError("Failed to initialize view. Program must now abort.");
		return false;
	}
	m_View->LoadURL("file:///Samples/ScreenShareHelper/ScreenShareHelper.html");
	m_UltralightMgr->SetViewToWindow(m_View->GetId(), m_Window->GetId());

	m_CPPTexture = std::make_shared<Texture>();
	if (!m_CPPTexture->Initialize(DirectoryHelper::GetAssetsDirectoryA() + "AIBowser.png"))
	{
		FatalError("Failed to load AIBowser.png.");
		return false;
	}

	return true;
}

DemoScreenShareHelper::~DemoScreenShareHelper()
{
	RemoveHook();
}

EZJSParm DemoScreenShareHelper::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "BeginShare")
	{
		StopSharing();
		RemoveHook();
		if (!SetHook())
		{
			FatalError("Failed to set hook for mouse capture.");
		}
		m_RectSelectionData.RectSelectionInProgress = true;
	}
	return EZJSParm();
}

void DemoScreenShareHelper::OnWindowDestroyStartCallback(int32_t windowId)
{
	if (m_PresentationWindow.expired() == false)
	{
		if (windowId == m_PresentationWindow->GetId())
		{
			StopSharing();
		}
	}
	if (m_Window.expired() == false)
	{
		if (windowId == m_Window->GetId())
		{
			WindowManager::DestroyAllWindows();
		}
	}
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoScreenShareHelper::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoScreenShareHelper::OnWindowResizeCallback(Window* pWindow)
{
	LOGINFO(strfmt("OnWindowResizeCallback [%s] W,H: [%d, %d]", pWindow->GetTitle().c_str(), pWindow->GetWidth(), pWindow->GetHeight()).c_str());
}

void DemoScreenShareHelper::OnPostRenderULViews()
{
	if (m_Window.expired())
	{
		return;
	}
}

void DemoScreenShareHelper::StopSharing()
{
	WindowManager::DestroyWindow(m_SelectedMonitorWindow);
	WindowManager::DestroyWindow(m_PresentationWindow);
	WindowManager::DestroyWindow(m_OutlineWindow);
}

void DemoScreenShareHelper::OnPreRenderULViews()
{
	m_TimeSinceLastUpdate += m_DeltaTime;
	if (m_PresentationWindow.expired()) //If presentation window isn't active, this is irrelevant since not presenting
	{
		return;
	}
	if (m_TimeSinceLastUpdate > c_TimeBetweenScreenUpdates)
	{
		m_TimeSinceLastUpdate = 0;
		if (m_ScreenCapper.CaptureRegion())
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ID3D11DeviceContext* pContext = m_Renderer.GetD3D()->m_Context.Get();

			HRESULT hr = pContext->Map(m_PresentationTexture->GetTextureResource(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(hr)) 
			{
				FatalError("Failed to map texture for updating.");
			}

			// Copy each row of data, accounting for RowPitch alignment
			const int width = m_PresentationTexture->m_Width;
			for (int y = 0; y < m_PresentationTexture->m_Height; ++y)
			{
				int rgbRow = (m_PresentationTexture->m_Height - y - 1);
				int textureRow = y;

				// Calculate source pointer to the current row in the RGBQUAD array
				unsigned char* pSourceRow = reinterpret_cast<unsigned char*>(m_ScreenCapper.m_Pixels + rgbRow * width);

				// Calculate destination pointer to the current row in the mapped resource
				unsigned char* pDestRow = reinterpret_cast<unsigned char*>(mappedResource.pData) + textureRow * mappedResource.RowPitch;

				// Copy the row, treating each pixel as 4 bytes (RGBA format)
				memcpy(pDestRow, pSourceRow, width * 4);
			}

			// Unmap the texture to apply the changes
			pContext->Unmap(m_PresentationTexture->GetTextureResource(), 0);

		}
	}
	WeakWrapper<RenderTargetContainer> renderTargetContainer = m_PresentationWindow->GetRenderTargetContainer();
	m_Renderer.ActivateRenderTarget(renderTargetContainer);

	//LOGINFO(strfmt("RT W/H [%d, %d]", renderTargetContainer->GetWidth(), renderTargetContainer->GetHeight()).c_str());
	//LOGINFO(strfmt("Presentation Texture W/H [%d, %d]", m_PresentationTexture->m_Width, m_PresentationTexture->m_Height).c_str());
	m_Renderer.DrawSprite(m_PresentationTexture.get(),
						  0,
						  0,
						  0, //Z
						  m_PresentationTexture->m_Width,
						  m_PresentationTexture->m_Height);

	if (GetAsyncKeyState(VK_F1))
	{
		RemoveHook();
		DebugBreak();
	}

	WeakWrapper<RenderTargetContainer> renderTargetContainerRect = m_OutlineWindow->GetRenderTargetContainer();
	m_Renderer.ActivateRenderTarget(renderTargetContainerRect);

	m_Renderer.DrawMesh2D(m_BorderRectMesh);

}

bool DemoScreenShareHelper::OnMouseMove(int x, int y)
{
	if (m_DragData.DragInProgress)
	{
		int deltaX = (x - m_DragData.StartMouseX);
		int deltaY = (y - m_DragData.StartMouseY);
		int winX = m_DragData.StartWindowX + deltaX;
		int winY = m_DragData.StartWindowY + deltaY;
		int regX = winX + m_OutlineThickness;
		int regY = winY + m_OutlineThickness;
		m_OutlineWindow->SetPosition(winX, winY);
		int regWidth = m_RectSelectionData.X2 - m_RectSelectionData.X1;
		int regHeight = m_RectSelectionData.Y2 - m_RectSelectionData.Y1;
		m_RectSelectionData.X1 = winX + m_OutlineThickness;
		m_RectSelectionData.Y1 = winY + m_OutlineThickness;
		m_RectSelectionData.X2 = m_RectSelectionData.X1 + regWidth;
		m_RectSelectionData.Y2 = m_RectSelectionData.Y1 + regHeight;
		m_ScreenCapper.AssignRegion(m_RectSelectionData.X1,
									m_RectSelectionData.Y1,
									regWidth, regHeight);
	}

	if (m_ResizingData.ResizeInProgress)
	{
		int deltaX = (x - m_ResizingData.MouseStartX);
		int deltaY = (y - m_ResizingData.MouseStartY);
		int newWidth = (m_ResizingData.WindowStartWidth);
		int newHeight = (m_ResizingData.WindowStartHeight);
		int winX = m_ResizingData.WindowStartX;
		int winY = m_ResizingData.WindowStartY;
		/*string msg = strfmt("DxDy [%d,%d] NwNh [%d, %d]", deltaX, deltaY, newWidth, newHeight);
		LOGINFO(msg.c_str());*/
		switch (m_ResizingData.Corner)
		{
		case Corner::TOPLEFT:
			winX += deltaX;
			winY += deltaY;
			newWidth -= deltaX;
			newHeight -= deltaY;
			break;
		case Corner::TOPRIGHT:
			winY += deltaY;
			newWidth += deltaX;
			newHeight -= deltaY;
			break;
		case Corner::BOTTOMLEFT:
			winX += deltaX;
			newWidth -= deltaX;
			newHeight += deltaY;
			break;
		case Corner::BOTTOMRIGHT:
			newWidth += deltaX;
			newHeight += deltaY;
			break;
		}
		m_OutlineWindow->SetPosition(winX, winY, newWidth, newHeight);
		//LOGINFO(strfmt("OutlineWindow xy [%d, %d] W/H [%d, %d]", winX, winY, newWidth, newHeight).c_str());
		int newRegionX = winX + m_OutlineThickness;
		int newRegionY = winY + m_OutlineThickness;
		int newRegionWidth = newWidth - m_OutlineThickness * 2;
		int newRegionHeight = newHeight - m_OutlineThickness * 2;

		m_ScreenCapper.AssignRegion(newRegionX, newRegionY, newRegionWidth, newRegionHeight);
		m_RectSelectionData.X1 = winX + m_OutlineThickness;
		m_RectSelectionData.Y1 = winY + m_OutlineThickness;
		m_RectSelectionData.X2 = m_RectSelectionData.X1 + newWidth - (m_OutlineThickness * 2);
		m_RectSelectionData.Y2 = m_RectSelectionData.Y1 + newHeight - (m_OutlineThickness * 2);

		//UpdateRectOutlineWindow();
		RECT presRect = m_PresentationWindow->GetRect();
		
		RebuildRectMesh(newWidth, newHeight);

		m_PresentationWindow->SetPosition(presRect.left, presRect.top, newRegionWidth, newRegionHeight);
		//LOGINFO(strfmt("PresentationWindow xy [%d, %d] W/H [%d, %d]", presRect.left, presRect.top, newRegionWidth, newRegionHeight).c_str());

		m_PresentationTexture = make_shared<Texture>();
		D3D11_TEXTURE2D_DESC desc{ 0 };
		desc.Width = newRegionWidth;
		desc.Height = newRegionHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (!m_PresentationTexture->Initialize(desc))
		{
			FatalError("Failed to initialize presentation texture.");
		}

	}

	if (m_RectSelectionData.RectSelectionInProgress)
	{
		string msg = strfmt("OnMouseMove [%d, %d]\n", x, y);
		OutputDebugStringA(msg.c_str());

		POINT cursorPos;
		if (!GetCursorPos(&cursorPos))
		{
			OutputDebugStringA("Failed to get cursor position.\n");
			return true;
		}

		// Step 2: Get the monitor handle for the current mouse position
		HMONITOR hMonitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
		if (hMonitor == nullptr)
		{
			OutputDebugStringA("Failed to get monitor handle.\n");
			return true;
		}

		// Step 3: Get monitor info
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(hMonitor, &monitorInfo)) {
			/*string msg = strfmt("Mouse is on monitor with coordinates...\n Left: %d\nTop: %d\nRight: %d\nBottom: %d\n",
								monitorInfo.rcMonitor.left,
								monitorInfo.rcMonitor.top,
								monitorInfo.rcMonitor.right,
								monitorInfo.rcMonitor.bottom);*/
								//OutputDebugStringA(msg.c_str());
								// Optional: You can check if the monitor is the primary display
			if (monitorInfo.dwFlags & MONITORINFOF_PRIMARY)
			{
				//OutputDebugStringA("This is the primary monitor.");
			}
			else
			{
				//OutputDebugStringA("This is the secondary monitor.");
			}
		}
		else
		{
			//OutputDebugStringA("Failed to get monitor information.");
		}

		if (m_RectSelectionData.HandleToMonitor != hMonitor)
		{
			if (m_SelectedMonitorWindow.expired() == false)
			{
				WindowManager::DestroyWindow(m_SelectedMonitorWindow->GetId());
			}
			if (m_SelectedMonitorWindow.expired())
			{
				WindowCreationParameters windowParms;
				windowParms.Width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
				windowParms.Height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
				windowParms.Style = WindowStyle::NoBorder | WindowStyle::Topmost | WindowStyle::TransparencyAllowed;
				windowParms.Title = "Screen Rect Staging Window";
				m_SelectedMonitorWindow = WindowManager::SpawnWindow(windowParms);
				if (m_SelectedMonitorWindow.expired())
				{
					FatalError("Failed to initialize staging window. Program must now abort.");
				}
				m_SelectedMonitorWindow->GetRenderTargetContainer()->SetBackgroundColor(0.5, 0.5, 0.5, 0.5);
				m_SelectedMonitorWindow->SetPosition(monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top);
			}
			m_RectSelectionData.HandleToMonitor = hMonitor;
		}

		return true;
	}
	return false;
}

bool DemoScreenShareHelper::OnMouseDown(int x, int y)
{
	if (m_RectSelectionData.RectSelectionInProgress)
	{
		m_RectSelectionData.X1 = x;
		m_RectSelectionData.Y1 = y;
		return true;
	}

	if (m_OutlineWindow.expired())
	{
		return false;
	}
	if (GetKeyState(VK_LMENU) & 0x8000 &&
		GetKeyState(VK_LCONTROL) & 0x8000) //left ctrl and alt
	{
		RECT rect = m_OutlineWindow->GetRect();
		if (x < rect.left || x > rect.right || y < rect.top || y > rect.bottom)
		{
			return false;
		}
		m_DragData.DragInProgress = true;
		m_DragData.StartWindowX = rect.left;
		m_DragData.StartWindowY = rect.top;
		m_DragData.StartMouseX = x;
		m_DragData.StartMouseY = y;
		return true;
	}
	if (GetKeyState(VK_LCONTROL) & 0x8000) //left ctrl
	{
		RECT rect = m_OutlineWindow->GetRect();
		if (x < rect.left || x > rect.right || y < rect.top || y > rect.bottom)
		{
			return false;
		}
		
		float topleftDistance = sqrt((x - rect.left) * (x - rect.left) + (y - rect.top) * (y - rect.top));
		float toprightDistance = sqrt((x - rect.right) * (x - rect.right) + (y - rect.top) * (y - rect.top));
		float botleftDistance = sqrt((x - rect.left) * (x - rect.left) + (y - rect.bottom) * (y - rect.bottom));
		float botrightDistance = sqrt((x - rect.right) * (x - rect.right) + (y - rect.bottom) * (y - rect.bottom));

		float closestCornerDistance = std::min(topleftDistance, toprightDistance);
		closestCornerDistance = std::min(closestCornerDistance, botleftDistance);
		closestCornerDistance = std::min(closestCornerDistance, botrightDistance);

		if (closestCornerDistance < 15.0f)
		{
			Corner corner = Corner::TOPLEFT;
			closestCornerDistance = topleftDistance;
			if (toprightDistance < topleftDistance)
			{
				corner = Corner::TOPRIGHT;
				closestCornerDistance = toprightDistance;
			}
			if (botleftDistance < closestCornerDistance)
			{
				corner = Corner::BOTTOMLEFT;
				closestCornerDistance = botleftDistance;
			}
			if (botrightDistance < closestCornerDistance)
			{
				corner = Corner::BOTTOMRIGHT;
			}

			m_ResizingData.Corner = corner;
			m_ResizingData.MouseStartX = x;
			m_ResizingData.MouseStartY = y;
			RECT rect = m_OutlineWindow->GetRect();
			m_ResizingData.WindowStartHeight = m_OutlineWindow->GetHeight();
			m_ResizingData.WindowStartWidth = m_OutlineWindow->GetWidth();
			m_ResizingData.WindowStartX = rect.left;
			m_ResizingData.WindowStartY = rect.top;
			m_ResizingData.ResizeInProgress = true;
			return true;
		}

		OutputDebugStringA(strfmt("Closest corner: %f\n", closestCornerDistance).c_str());

		/*m_DragData.DragInProgress = true;
		m_DragData.StartWindowX = rect.left;
		m_DragData.StartWindowY = rect.top;
		m_DragData.StartMouseX = x;
		m_DragData.StartMouseY = y;*/
		return false;
	}

	return false;
	string msg = strfmt("OnMouseDown [%d, %d]\n", x, y);
	OutputDebugStringA(msg.c_str());
}

bool DemoScreenShareHelper::OnMouseUp(int x, int y)
{
	if (m_RectSelectionData.RectSelectionInProgress == true)
	{
		OutputDebugStringA("RectSelectionInProgess=true\n");
		m_RectSelectionData.RectSelectionInProgress = false;
		m_RectSelectionData.X2 = x;
		m_RectSelectionData.Y2 = y;

		WindowManager::DestroyWindow(m_SelectedMonitorWindow);  //Shouldn't exist here, but to be sure delete
		if (m_RectSelectionData.X1 > m_RectSelectionData.X2)
		{
			std::swap(m_RectSelectionData.X1, m_RectSelectionData.X2);
		}
		if (m_RectSelectionData.Y1 > m_RectSelectionData.Y2)
		{
			std::swap(m_RectSelectionData.Y1, m_RectSelectionData.Y2);
		}

		WindowManager::DestroyWindow(m_PresentationWindow); //Shouldn't exist here, but to be sure delete

		WindowCreationParameters windowParms;
		windowParms.Width = m_RectSelectionData.X2 - m_RectSelectionData.X1;
		windowParms.Height = m_RectSelectionData.Y2 - m_RectSelectionData.Y1;
		windowParms.Style = WindowStyle::ExitButton;
		windowParms.Title = "Screen Share Presentation Window";
		m_PresentationWindow = WindowManager::SpawnWindow(windowParms);
		if (m_PresentationWindow.expired())
		{
			FatalError("Failed to initialize presentation window. Program must now abort.");
		}
		m_PresentationWindow->GetRenderTargetContainer()->SetBackgroundColor(0, 0, 0, 1);
		RECT presRect = m_PresentationWindow->GetRect();
		if (presRect.top < 20)
		{
			m_PresentationWindow->SetPosition(presRect.left, 20);
		}
		m_PresentationTexture = make_shared<Texture>();
		D3D11_TEXTURE2D_DESC desc{ 0 };
		desc.Width = windowParms.Width;
		desc.Height = windowParms.Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (!m_PresentationTexture->Initialize(desc))
		{
			FatalError("Failed to initialize presentation texture.");
		}
		m_ScreenCapper.AssignRegion(m_RectSelectionData.X1, m_RectSelectionData.Y1, windowParms.Width, windowParms.Height);

		{
			WindowManager::DestroyWindow(m_OutlineWindow);
			UpdateRectOutlineWindow();
		}
		return true;
	}
	
	if (m_DragData.DragInProgress)
	{
		m_DragData.DragInProgress = false;
		OutputDebugStringA("DragInProgress=false\n");
		return true;
	}

	if (m_ResizingData.ResizeInProgress)
	{
		m_ResizingData.ResizeInProgress = false;
		return true;
	}

	string msg = strfmt("OnMouseUp [%d, %d]\n", x, y);
	OutputDebugStringA(msg.c_str());
	return false;
}

void DemoScreenShareHelper::UpdateRectOutlineWindow()
{
	float outlineThickness = m_OutlineThickness;
	int outlineWindowWidth = (m_RectSelectionData.X2 - m_RectSelectionData.X1) + outlineThickness * 2;
	int outlineWindowHeight = (m_RectSelectionData.Y2 - m_RectSelectionData.Y1) + outlineThickness * 2;

	WindowCreationParameters windowParms;
	windowParms.Width = outlineWindowWidth;
	windowParms.Height = outlineWindowHeight;
	windowParms.Style = WindowStyle::NoBorder | WindowStyle::TransparencyAllowed | WindowStyle::Topmost;
	windowParms.Title = "Screen Share Border Window";
	//RemoveHook();
	m_OutlineWindow = WindowManager::SpawnWindow(windowParms);
	m_OutlineWindow->ToggleClickthrough(true);
	auto rt = m_OutlineWindow->GetRenderTargetContainer();
	rt->SetBackgroundColor(0, 0, 0, 0);
	m_OutlineWindow->SetPosition(m_RectSelectionData.X1 - outlineThickness, m_RectSelectionData.Y1 - outlineThickness);

	m_OutlineWindow->SetPosition(m_RectSelectionData.X1 - outlineThickness, m_RectSelectionData.Y1 - outlineThickness,
								 outlineWindowWidth, outlineWindowHeight);


	m_BorderRectMesh.PipelineState = m_Renderer.GetPipelineState("2D");
	if (m_BorderRectMesh.Texture == nullptr)
	{
		m_BorderRectMesh.Texture = make_shared<Texture>();
		if (!m_BorderRectMesh.Texture->Initialize(DirectoryHelper::GetAssetsDirectoryA() + "Red.png"))
		{
			FatalError("Failed to load Red.png.");
		}
	}
	
	RebuildRectMesh(outlineWindowWidth, outlineWindowHeight);
}

void DemoScreenShareHelper::RebuildRectMesh(int width, int height)
{
	float thickness = m_OutlineThickness;
	auto& mesh = m_BorderRectMesh.Mesh2D;
	vector<Vertex_2D> vertices = {
		Vertex_2D(0,     0, 0, 0, 0), //TOPLEFT x, y, z, u, v   
		Vertex_2D(width, 0, 0, 0, 0), //TOPRIGHT
		Vertex_2D(0,     thickness, 0, 0, 0), //TOPLEFT botpart
		Vertex_2D(width, thickness, 0, 0, 0), //TOPRIGHT botpart
		Vertex_2D(0,     height - thickness, 0, 0, 0), //bottomleft top part
		Vertex_2D(width, height - thickness, 0, 0, 0), //botright top part
		Vertex_2D(0,     height, 0, 0, 0), //bottomleft x, y, z, u, v   
		Vertex_2D(width, height, 0, 0, 0), //bottomright
		
		Vertex_2D(0,     thickness, 0, 0, 0), //left topleft part
		Vertex_2D(thickness, thickness, 0, 0, 0), //left topright part
		Vertex_2D(0,     height - thickness, 0, 0, 0), //left bottomleft part  
		Vertex_2D(thickness, height - thickness, 0, 0, 0), //left bottomright part

		Vertex_2D(width - thickness,     thickness, 0, 0, 0), //right topleft part
		Vertex_2D(width, thickness, 0, 0, 0), //right topright part
		Vertex_2D(width - thickness,     height - thickness, 0, 0, 0), //right bottomleft part  
		Vertex_2D(width, height - thickness, 0, 0, 0), //right bottomright part


	};
	vector<uint32_t> indices = {
		0, 1, 2,
		2, 1, 3,
		4, 5, 6,
		6, 5, 7,
		8, 9, 10,
		10, 9, 11,
		12, 13, 14,
		14, 13, 15
	};
	if (!mesh.Initialize(vertices, indices))
	{
		FatalError("Failed to initialize border mesh.");
	}
}

