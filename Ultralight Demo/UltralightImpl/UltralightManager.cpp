#include <PCH.h>
#include "UltralightManager.h"
#include "../Engine.h"
#include "../Graphics/Misc/PixelColor.h"
UltralightManager* UltralightManager::s_Instance = nullptr;

bool UltralightManager::Initialize()
{
	if (s_Instance != nullptr)
	{
		FatalError("UltralightManager attempted to be initialized more than once. UltralightManager is a singleton.");
		return false;
	}

	s_Instance = this;

	//Ultralight instance initialization
	ul::Config config;
	//config.resource_path = ul::String(std::string(DirectoryHelper::GetExecutableDirectoryA() + "resources").c_str());
	config.cache_path = ul::String(std::string(DirectoryHelper::GetExecutableDirectoryA() + "resources").c_str());;// ul::String(std::string(DirectoryHelper::GetExecutableDirectoryA() + "resources").c_str());
	config.face_winding = ul::FaceWinding::Clockwise;
	config.max_update_time = 1.0f / 30.0f;
	config.bitmap_alignment = 0;

	ul::Platform::instance().set_config(config);

	m_Logger = std::make_unique<LoggerDefault>(DirectoryHelper::GetExecutableDirectoryA() + "log.txt");
	ul::Platform::instance().set_logger(m_Logger.get());

	m_FontLoader = std::make_unique<FontLoaderWin>();
	ul::Platform::instance().set_font_loader(m_FontLoader.get());

	m_FileSystem = std::make_unique<FileSystemWin>((DirectoryHelper::GetWebDirectory()).c_str());
	ul::Platform::instance().set_file_system(m_FileSystem.get());

	m_GPUDriver = std::make_unique<GPUDriverD3D11>();
	ul::Platform::instance().set_gpu_driver(m_GPUDriver.get());

	m_Clipboard = std::make_unique<ClipboardWin>();
	ul::Platform::instance().set_clipboard(m_Clipboard.get());

	m_UltralightRenderer = ul::Renderer::Create();
	if (m_UltralightRenderer.get() == nullptr)
	{
		ErrorHandler::LogCriticalError("Failed to initialize ultralight Renderer.");
		return false;
	}

	return true;
}

void UltralightManager::Shutdown()
{
	m_UltralightRenderer = nullptr;
}

void UltralightManager::UpdateViews()
{
	m_UltralightRenderer->Update();
	m_UltralightRenderer->Render();
	m_GPUDriver->DrawCommandList();
}

ul::Renderer* UltralightManager::GetRendererPtr()
{
	return m_UltralightRenderer.get();
}

void UltralightManager::RemoveViewFromWindow(int32_t viewId, int32_t windowId)
{
	//TODO: Add error checking
	auto windowIter = m_WindowIdToViewIdMap.find(windowId);
	if (windowIter != m_WindowIdToViewIdMap.end())
	{
		windowIter->second.erase(viewId);
	}
	auto viewIter = m_ViewsMap.find(viewId);
	if (viewIter != m_ViewsMap.end())
	{
		viewIter->second->SetToWindow(-1);
	}
	Engine* pEngine = Engine::GetInstance();
	assert(pEngine != nullptr);
	Window* pWindow = pEngine->GetWindowFromId(windowId);
	assert(pWindow != nullptr);
	auto& viewList = pWindow->m_UltralightViewsSorted;
	for (auto listIter = viewList.begin(); listIter != viewList.end(); listIter++)
	{
		if (listIter->get()->GetId() == viewId)
		{
			viewList.erase(listIter);
			break;
		}
	}
}

void UltralightManager::SetViewToWindow(int32_t viewId, int32_t windowId)
{
	//TODO: Add error checking
	auto viewIter = m_ViewsMap.find(viewId);
	if (viewIter == m_ViewsMap.end())
	{
		return; //This shouldn't happen doesn't make sense
	}
	auto pView = viewIter->second;
	if (pView->m_WindowId != -1)
	{
		RemoveViewFromWindow(viewId, pView->m_WindowId);
	}

	auto windowIter = m_WindowIdToViewIdMap.find(windowId);
	if (windowIter != m_WindowIdToViewIdMap.end())
	{
		windowIter->second.insert(viewId);
		auto pView = viewIter->second;
		pView->SetToWindow(windowId);
	}
	
	Engine* pEngine = Engine::GetInstance();
	assert(pEngine != nullptr);
	Window* pWindow = pEngine->GetWindowFromId(windowId);
	assert(pWindow != nullptr);
	auto& viewList = pWindow->m_UltralightViewsSorted;
	bool viewInserted = false;
	for (auto listIter = viewList.begin(); listIter != viewList.end(); listIter++)
	{
		if (listIter->get()->m_Position.z >= pView->m_Position.z)
		{
			viewList.insert(listIter, pView);
			viewInserted = true;
			break;
		}
	}
	if (viewInserted == false)
	{
		viewList.insert(viewList.end(), pView);
	}
}

void UltralightManager::RegisterWindow(std::shared_ptr<Window> pWindow)
{
	m_WindowIdToViewIdMap.insert(make_pair(pWindow->GetId(), set<int32_t>()));
	m_WindowIdToWindowPtrMap.insert(make_pair(pWindow->GetId(), pWindow));
}

void UltralightManager::RemoveWindowId(int32_t windowId)
{
	//TODO: Add error checking
	m_WindowIdToViewIdMap.erase(windowId);
	m_WindowIdToWindowPtrMap.erase(windowId);
}

std::shared_ptr<UltralightView> UltralightManager::CreateUltralightView(UltralightViewCreationParameters parms)
{
	std::shared_ptr<UltralightView> pView = std::make_shared<UltralightView>();
	if (!pView->Initialize(parms))
	{
		ErrorHandler::LogCriticalError("Failed to create Ultralight View.");
		return nullptr;
	}
	m_ViewsMap.insert(std::make_pair(pView->GetId(), pView));
	return pView;
}

UltralightManager* UltralightManager::GetInstance()
{
	return s_Instance;
}

vector<shared_ptr<UltralightView>> UltralightManager::GetViewsForWindow(int32_t windowId)
{
	//TODO: Error checking and optimize these unnecessary lookups
	vector<shared_ptr<UltralightView>> views;
	auto windowIter = m_WindowIdToViewIdMap.find(windowId);
	if (windowIter != m_WindowIdToViewIdMap.end())
	{
		for (auto viewId : windowIter->second)
		{
			auto viewIter = m_ViewsMap.find(viewId);
			if (viewIter != m_ViewsMap.end())
			{
				views.push_back(viewIter->second);
			}
		}
	}
	return views;
}

std::shared_ptr<UltralightView> UltralightManager::GetViewFromId(int32_t viewId)
{
	//TODO: Error checking
	return m_ViewsMap[viewId];
}

GPUDriverD3D11* UltralightManager::GetGPUDriver()
{
	return m_GPUDriver.get();
}

unordered_map<int32_t, shared_ptr<UltralightView>> UltralightManager::GetViews()
{
	return m_ViewsMap;
}

void UltralightManager::RefreshViewDisplaysForAnimations()
{
	for (auto view : m_ViewsMap)
	{
		m_UltralightRenderer->RefreshDisplay(view.second->GetView()->display_id());
	}
}

bool UltralightManager::FireKeyboardEvent(KeyboardEvent* keyboardEvent)
{
	int32_t windowId = keyboardEvent->GetWindowId();
	std::shared_ptr<Window> pWindow = m_WindowIdToWindowPtrMap[windowId];

	if (pWindow->m_FocusedUltralightView != nullptr)
	{
		pWindow->m_FocusedUltralightView->FireKeyboardEvent(keyboardEvent->ToUltralightKeyboardEvent());
		return true;
	}

	return false;
}

bool UltralightManager::FireMouseEvent(MouseEvent* mouseEvent)
{
	if (mouseEvent->GetType() == MouseEvent::Type::MouseMoveRaw)
		return false;

	int32_t windowId = mouseEvent->GetWindowId();
	std::shared_ptr<Window> pWindow = m_WindowIdToWindowPtrMap[windowId];

	if (mouseEvent->GetType() == MouseEvent::Type::MouseUp)
	{
		if (pWindow->m_FocusedUltralightView != nullptr)
		{
			pWindow->m_FocusedUltralightView->FireMouseEvent(mouseEvent->ToUltralightMouseEvent());
			return true;
		}
	}

	auto pViews = pWindow->m_UltralightViewsSorted;

	bool dispatchedToHtml = false;
	for (auto iter = pViews.begin(); iter != pViews.end(); iter++)
	{
		shared_ptr<UltralightView> pView = *iter;
		if (pView->IsVisible() == false)
		{
			continue;
		}
		if (pView->IsInputEnabled() == false)
		{
			continue;
		}

		DirectX::XMFLOAT3 pos = pView->GetPosition();
		Matrix worldMatrix = DirectX::XMMatrixScaling(pView->GetWidth(), pView->GetHeight(), 1.0f) *
							 DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		
		Vector2 mouseCoords(mouseEvent->GetPosX(), mouseEvent->GetPosY());

		worldMatrix = worldMatrix.Invert();

		Vector2 result = result.Transform(mouseCoords, worldMatrix);
		if (result.x >= 0 && result.x <= 1 &&
			result.y >= 0 && result.y <= 1) //Was mouse event inside the rectangle of the html view?
		{
			if (mouseEvent->GetType() == MouseEvent::Type::MouseDown || mouseEvent->GetType() == MouseEvent::Type::MouseUp)
			{
				int x = result.x * pView->GetWidth();
				int y = result.y * pView->GetHeight();

				PixelColor c = pView->GetPixelColor(x, y);
				if (c.rgba[3] == 0)
				{
					continue;
				}
			}

			pView->FireMouseEvent(mouseEvent->ToUltralightMouseEvent());
			switch (mouseEvent->GetType())
			{
			case (MouseEvent::Type::MouseDown):
			{
				if (pWindow->m_FocusedUltralightView != nullptr)
				{
					if (pWindow->m_FocusedUltralightView != pView)
					{
						pWindow->m_FocusedUltralightView->m_NativeView->Unfocus();
					}
				}
				pWindow->m_FocusedUltralightView = pView;
				pView->m_NativeView->Focus();
				return true;
			}
			}
			dispatchedToHtml = true;
			//return true;
			if (mouseEvent->GetType() != MouseEvent::Type::MouseMove)
			{
				return true;
			}
		}
		else
		{
		}
	}
	if (mouseEvent->GetType() == MouseEvent::Type::MouseDown)
	{
		if (pWindow->m_FocusedUltralightView != nullptr)
		{
			pWindow->m_FocusedUltralightView->m_NativeView->Unfocus();
		}
		pWindow->m_FocusedUltralightView = nullptr;
	}
	return dispatchedToHtml;
}

bool UltralightManager::FireScrollEvent(ScrollEvent* scrollEvent)
{
	int32_t windowId = scrollEvent->GetWindowId();
	std::shared_ptr<Window> pWindow = m_WindowIdToWindowPtrMap[windowId];

	if (pWindow->m_FocusedUltralightView != nullptr)
	{
		pWindow->m_FocusedUltralightView->FireScrollEvent(scrollEvent->ToUltralightScrollEvent());
		return true;
	}

	return false;
}

UltralightManager::~UltralightManager()
{
	m_UltralightRenderer = nullptr;
}