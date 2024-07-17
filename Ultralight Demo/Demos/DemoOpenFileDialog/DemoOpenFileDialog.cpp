#include "PCH.h"
#include "DemoOpenFileDialog.h"
#include "../Misc/CursorManager.h"
#include <ShlObj.h>
#include <filesystem>
#pragma comment(lib, "shell32.lib")
namespace fs = std::filesystem;

bool DemoOpenFileDialog::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "OpenFileDialog Demo Main Window";
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

	m_PrimaryView = m_UltralightMgr.CreateUltralightView(parms);
	m_PrimaryView->LoadURL("file:///Samples/OpenFileDialog/Startup.html");
	m_UltralightMgr.SetViewToWindow(m_PrimaryView->GetId(), m_PrimaryWindow->GetId());
}

bool DemoOpenFileDialog::Tick()
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

EZJSParm DemoOpenFileDialog::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "OpenFileDialogLoaded")
	{
		CHAR my_documents[MAX_PATH];
		HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);
		if (result == S_OK)
		{
			std::string path(my_documents);
			shared_ptr<UltralightView> pView = m_UltralightMgr.GetViewFromId(viewId);
			EZJSParm outReturnVal;
			string outException;
			bool result = pView->CallJSFnc("AddQuickAccessLocation",
											{ "My Documents", path},
											outReturnVal,
											outException);
			if (result == false)
			{
				ErrorHandler::LogCriticalError("Failed to add quick access path to open file dialog.");
			}
		}
		return EZJSParm();
	}
	if (eventName == "OpenFileDialog_OpenFolder")
	{
		if (parameters.size() == 1)
		{
			if (parameters[0].GetType() == EZJSParm::String)
			{
				string path = parameters[0].AsString();
				vector<EZJSParm> directoryEntries;
				for (const auto& entry : fs::directory_iterator(path))
				{
					directoryEntries.push_back(entry.path().string());
					OutputDebugStringA(strfmt("Directory: %s\n", entry.path().string().c_str()).c_str());
				}


				shared_ptr<UltralightView> pView = m_UltralightMgr.GetViewFromId(viewId);
				EZJSParm outReturnVal;
				string outException;
				bool result = pView->CallJSFnc("UpdateDirectoryLocationAndEntries",
											   { path, directoryEntries },
											   outReturnVal,
											   outException);
				if (result == false)
				{
					ErrorHandler::LogCriticalError("Failed to update directory location and entries.");
				}

				OutputDebugStringA("Opening folder...");
				OutputDebugStringA(path.c_str());
			}
		}
		return EZJSParm();
	}
	if (eventName == "OpenFileDialog")
	{
		if (m_OpenFileDialogWindow == nullptr)
		{
			WindowCreationParameters windowParms;
			windowParms.Width = 800;
			windowParms.Height = 600;
			windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
			windowParms.Title = "OpenFileDialog File Selection";
			windowParms.ParentWindow = m_PrimaryWindow->GetHWND(); //By setting the parent, this window will always be on top.
			shared_ptr<Window> pWindow = SpawnWindow(windowParms);
			m_OpenFileDialogWindow = pWindow;
			if (m_OpenFileDialogWindow == nullptr)
			{
				FatalError("Failed to initialize open file dialog window. Program must now abort.");
			}
		}
		if (m_OpenFileDialogView == nullptr)
		{
			UltralightViewCreationParameters parms;
			parms.Width = m_OpenFileDialogWindow->GetWidth();
			parms.Height = m_OpenFileDialogWindow->GetHeight();
			parms.IsAccelerated = false;
			parms.ForceMatchWindowDimensions = true;
			parms.IsTransparent = true;

			m_OpenFileDialogView = m_UltralightMgr.CreateUltralightView(parms);
			m_OpenFileDialogView->LoadURL("file:///Samples/OpenFileDialog/OpenFileDialog.html");
		}
		m_UltralightMgr.SetViewToWindow(m_OpenFileDialogView->GetId(), m_OpenFileDialogWindow->GetId());
		m_OpenFileDialogWindow->Show();
		return EZJSParm();
	}

	return EZJSParm();
}

void DemoOpenFileDialog::OnWindowDestroyCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	if (m_OpenFileDialogWindow != nullptr)
	{
		if (windowId == m_OpenFileDialogWindow->GetId())
		{
			m_OpenFileDialogWindow = nullptr;
		}
	}
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		if (pView == m_OpenFileDialogView) //Technically this is inefficient. Could keep the view alive and just reassign it to window, but this is simpler to manage.
		{
			m_OpenFileDialogView = nullptr;
		}
		m_UltralightMgr.DestroyView(pView);
	}
}

void DemoOpenFileDialog::OnWindowResizeCallback(Window* pWindow)
{
}

