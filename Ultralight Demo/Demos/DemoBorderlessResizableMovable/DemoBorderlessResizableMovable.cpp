#include "PCH.h"
#include "DemoBorderlessResizableMovable.h"
#include "../Misc/CursorManager.h"

bool DemoBorderlessResizableMovable::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::NoBorder;
	windowParms.Title = "Default Title";
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	m_PrimaryWindow = pWindow;
	if (m_PrimaryWindow == nullptr)
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

	shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/BorderlessWindow/BorderlessWindow.html");
	m_UltralightMgr.SetViewToWindow(pView->GetId(), pWindow->GetId());
}

bool DemoBorderlessResizableMovable::Tick()
{
	//Process Input Events
	auto& keyboard = m_InputController.m_Keyboard;
	auto& mouse = m_InputController.m_Mouse;
	while (mouse.EventBufferIsEmpty() == false)
	{
		MouseEvent mouseEvent = mouse.ReadEvent();

		if (m_WindowDragInfo.DragInProgress) //To allow dragging the borderless window
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
								 m_WindowDragInfo.DragStartWindowPosition.y + deltaY, 
								 0, 0, SWP_NOSIZE);
				}
			}
		}

		bool dispatchedToHtml = m_UltralightMgr.FireMouseEvent(&mouseEvent);
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
		bool dispatchedToHtml = m_UltralightMgr.FireScrollEvent(&scrollEvent);
	}
	while (keyboard.EventBufferIsEmpty() == false)
	{
		KeyboardEvent keyboardEvent = keyboard.ReadEvent();
		bool dispatchedtoHtml = m_UltralightMgr.FireKeyboardEvent(&keyboardEvent);
	}

	return true;
}

EZJSParm DemoBorderlessResizableMovable::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	shared_ptr<UltralightView> pView = m_UltralightMgr.GetViewFromId(viewId);
	assert(pView != nullptr);

	if (eventName == "RequestTitle") //BorderlessWindow.html calls this on load to fill span for title
	{
		return "BorderlessResizableMovable Demo";
	}

	if (eventName == "CloseWindow")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		Window* pWindow = GetWindowFromId(windowId);
		PostMessage(pWindow->GetHWND(), WM_CLOSE, NULL, NULL);
		return nullptr;
	}

	if (eventName == "MaximizeRestore")
	{
		int32_t windowId = pView->GetWindowId();
		assert(windowId != -1);
		Window* pWindow = GetWindowFromId(windowId);
		if (pWindow->IsWindowMaximized())
		{
			pWindow->Restore();
		}
		else
		{
			pWindow->Maximize();
		}
		return nullptr;
	}

	if (eventName == "BeginWindowDrag")
	{
		auto pView = m_UltralightMgr.GetViewFromId(viewId);
		int32_t windowId = pView->GetWindowId();
		auto pWindow = m_WindowIdToWindowInstanceMap[windowId];
		if (pWindow->IsWindowMaximized() == false)
		{
			//pWindow->StartDrag();
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
		}
		return EZJSParm();
	}

	return EZJSParm();
}

void DemoBorderlessResizableMovable::OnWindowDestroyCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr.DestroyView(pView);
	}
}

void DemoBorderlessResizableMovable::OnWindowResizeCallback(Window* pWindow)
{
	for (auto ulView : pWindow->GetSortedUltralightViews())
	{
		EZJSParm outReturnVal;
		string outException;
		ulView->CallJSFnc("UpdateDimensions",
						  { pWindow->GetWidth(), pWindow->GetHeight() },
						  outReturnVal,
						  outException);
	}
}