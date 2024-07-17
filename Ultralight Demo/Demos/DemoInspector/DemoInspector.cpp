#include "PCH.h"
#include "DemoInspector.h"
#include "../Misc/CursorManager.h"

bool DemoInspector::Startup()
{
	//Main view creation
	WindowCreationParameters mainWindowParms;
	mainWindowParms.Width = 800;
	mainWindowParms.Height = 600;
	mainWindowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	mainWindowParms.Title = "Default Title";
	shared_ptr<Window> pWindow = SpawnWindow(mainWindowParms);
	m_PrimaryWindow = pWindow;
	if (m_PrimaryWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters mainViewParms;
	mainViewParms.Width = pWindow->GetWidth();
	mainViewParms.Height = pWindow->GetHeight();
	mainViewParms.IsAccelerated = false; //Use GPU Rendering?
	mainViewParms.ForceMatchWindowDimensions = true;
	mainViewParms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(mainViewParms);
	pView->LoadURL("http://www.google.com");
	m_UltralightMgr.SetViewToWindow(pView->GetId(), pWindow->GetId());

	//Inspector view creation
	WindowCreationParameters inspectorWindowParms;
	inspectorWindowParms.Width = 800;
	inspectorWindowParms.Height = 600;
	inspectorWindowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	inspectorWindowParms.Title = "Inspector Window";
	shared_ptr<Window> pInspectorWindow = SpawnWindow(inspectorWindowParms);
	if (pInspectorWindow == nullptr)
	{
		FatalError("Failed to initialize inspector window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters inspectorViewParms;
	inspectorViewParms.Width = pInspectorWindow->GetWidth();
	inspectorViewParms.Height = pInspectorWindow->GetHeight();
	inspectorViewParms.IsAccelerated = false; //Use GPU Rendering?
	inspectorViewParms.ForceMatchWindowDimensions = true;
	inspectorViewParms.IsTransparent = false;
	inspectorViewParms.InspectionTarget = pView;

	shared_ptr<UltralightView> pInspectorView = m_UltralightMgr.CreateUltralightView(inspectorViewParms);
	//Note that m_UltralightMgr.CreateUltralightView will call CreateLocalInspectorView on the inspection target when that parameter is passed into the creation parms.
	m_UltralightMgr.SetViewToWindow(pInspectorView->GetId(), pInspectorWindow->GetId());

	return true;

}

bool DemoInspector::Tick()
{
	//Process Input Events
	auto& keyboard = m_InputController.m_Keyboard;
	auto& mouse = m_InputController.m_Mouse;
	while (mouse.EventBufferIsEmpty() == false)
	{
		MouseEvent mouseEvent = mouse.ReadEvent();
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

	RenderFrame();

	return true;
}

EZJSParm DemoInspector::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoInspector::OnWindowDestroyCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr.DestroyView(pView);
	}
}

void DemoInspector::OnWindowResizeCallback(Window* pWindow)
{
}

