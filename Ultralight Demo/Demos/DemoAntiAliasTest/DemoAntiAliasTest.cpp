#include "PCH.h"
#include "DemoAntiAliasTest.h"
#include "../Misc/CursorManager.h"

bool DemoAntiAliasTest::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 330;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	

	UltralightViewCreationParameters parms;
	parms.Width = windowParms.Width;
	parms.Height = windowParms.Height;
	parms.IsAccelerated = true;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	for (int sampleCount = 1; sampleCount <= 8; sampleCount *= 2)
	{
		if (sampleCount > 1)
		{
			if (m_Renderer.GetD3D()->IsSampleCountSupported(sampleCount) == false)
			{
				continue;
			}
		}

		windowParms.Title = "SampleCount = ";
		windowParms.Title += std::to_string(sampleCount);

		shared_ptr<Window> pWindow = SpawnWindow(windowParms);
		if (pWindow == nullptr)
		{
			FatalError("Failed to initialize window. Program must now abort.");
			return false;
		}

		parms.SampleCount = sampleCount;
		shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
		pView->LoadURL("file:///Samples/AntiAliasTest/AntiAliasTest.html");
		m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());
	}
}

bool DemoAntiAliasTest::Tick()
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

EZJSParm DemoAntiAliasTest::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoAntiAliasTest::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoAntiAliasTest::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoAntiAliasTest::OnWindowResizeCallback(Window* pWindow)
{
}

