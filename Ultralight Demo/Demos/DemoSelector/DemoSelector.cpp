#include "PCH.h"
#include "DemoSelector.h"
#include "../Misc/CursorManager.h"

bool DemoSelector::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo Selector";
	m_PrimaryWindow = SpawnWindow(windowParms);
	if (m_PrimaryWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_PrimaryWindow->GetWidth();
	parms.Height = m_PrimaryWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/DemoSelector/DemoSelector.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), m_PrimaryWindow->GetId());
}

bool DemoSelector::Tick()
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

EZJSParm DemoSelector::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "SelectDemo")
	{
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::String);
		string demoName = parameters[0].AsString();
		if (demoName == "DemoBasic")
		{
			m_SelectedDemo = DemoId::DemoBasic;
		}
		if (demoName == "DemoBorderlessResizable")
		{
			m_SelectedDemo = DemoId::DemoBorderlessResizable;
		}
		if (demoName == "DemoBorderlessResizableMovable")
		{
			m_SelectedDemo = DemoId::DemoBorderlessResizableMovable;
		}
		if (demoName == "DemoCPPTextureInBrowser")
		{
			m_SelectedDemo = DemoId::DemoCPPTextureInBrowser;
		}
		if (demoName == "DemoInspector")
		{
			m_SelectedDemo = DemoId::DemoInspector;
		}
		if (demoName == "DemoJSCPPCommunication")
		{
			m_SelectedDemo = DemoId::DemoJSCPPCommunication;
		}
		if (demoName == "DemoOpenFileDialog")
		{
			m_SelectedDemo = DemoId::DemoOpenFileDialog;
		}
		if (demoName == "DemoTransparent")
		{
			m_SelectedDemo = DemoId::DemoTransparent;
		}
		if (m_SelectedDemo != DemoId::None)
		{
			m_PrimaryWindow->Close();
		}
	}
	return EZJSParm();
}

void DemoSelector::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoSelector::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoSelector::OnWindowResizeCallback(Window* pWindow)
{
}

