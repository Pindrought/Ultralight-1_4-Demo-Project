#include "PCH.h"
#include "DemoCPPTextureInBrowser.h"
#include "../Misc/CursorManager.h"

bool DemoCPPTextureInBrowser::InitializeUltralight()
{
	m_CustomGPUImpl = std::make_shared<CustomGPUImpl>();
	m_FileSystemImpl = std::make_shared<CustomFileSystem>(DirectoryHelper::GetWebDirectory().c_str());
	UltralightOverrides overrides;
	overrides.GPUDriver = m_CustomGPUImpl;
	overrides.FileSystem = m_FileSystemImpl;
	if (!m_UltralightMgr.Initialize(&overrides))
	{
		FatalError("DemoCPPTextureInBrowser ultralight initialization failed.");
		return false;
	}
	return true;
}

bool DemoCPPTextureInBrowser::Startup()
{
	shared_ptr<Texture> aiMarioTexture = std::make_shared<Texture>();
	if (!aiMarioTexture->Initialize("AIMario.png"))
	{
		return false;
	}
	m_CPPTexturesForBrowserMap.insert(make_pair("AIMarioTexture", aiMarioTexture));
	shared_ptr<Texture> aiBowserTexture = std::make_shared<Texture>();
	if (!aiBowserTexture->Initialize("AIBowser.png"))
	{
		return false;
	}
	m_CPPTexturesForBrowserMap.insert(make_pair("AIBowserTexture", aiBowserTexture));

	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
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
	parms.IsAccelerated = true; //Have to use GPU rendering for this to work.
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	shared_ptr<UltralightView> pView = m_UltralightMgr.CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/CPPTextureInBrowser/CPPTextureInBrowser.html");
	m_UltralightMgr.SetViewToWindow(pView->GetId(), pWindow->GetId());
}

bool DemoCPPTextureInBrowser::Tick()
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

	return true;
}

EZJSParm DemoCPPTextureInBrowser::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "GetCPPTexturePath")
	{
		OutputDebugStringA("GenerateCPPTexture\n");
		if (parameters.size() == 1)
		{
			if (parameters[0].GetType() == EZJSParm::String)
			{
				string textureName = parameters[0].AsString();
				auto iter = m_CPPTexturesForBrowserMap.find(textureName);
				if (iter != m_CPPTexturesForBrowserMap.end())
				{
					static uint32_t pixelId = 4294921702;
					//pixelId += 1;
					std::string stagingTexturePath = "__STAGINGTEXTURE__" + std::to_string(pixelId) + ".png";

					TextureOverwriteData data;
					data.Texture = iter->second;
					data.Alias = textureName;
					data.PixelUID = pixelId;
					m_CustomGPUImpl->QueueCPPTextureOverwrite(data);
					return stagingTexturePath;
				}
			}
		}
	}
	return EZJSParm();
}

void DemoCPPTextureInBrowser::OnWindowDestroyCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr.DestroyView(pView);
	}
}

void DemoCPPTextureInBrowser::OnWindowResizeCallback(Window* pWindow)
{
}

