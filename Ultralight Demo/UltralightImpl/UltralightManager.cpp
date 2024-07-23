#include <PCH.h>
#include "UltralightManager.h"
#include "../Engine.h"
#include "../Graphics/Misc/PixelColor.h"
#include "GPUimpl/GPUDriverD3D11.h"

shared_ptr<UltralightManager> UltralightManager::s_Instance = nullptr;

bool UltralightManager::Initialize(UltralightOverrides* ultralightOverrides)
{
	UltralightOverrides ulOverrides;
	if (ultralightOverrides != nullptr)
	{
		ulOverrides = *ultralightOverrides;
	}

	if (m_UltralightRenderer.get() != nullptr) //Already initialized?
	{
		IGPUDriverD3D11::OldReservedEntries oldEntries = m_GPUDriver->GetOutstandingReservedIds();
		//Need to create new gpu driver and retarget to our new instance
		if (ulOverrides.GPUDriver == nullptr) //Default behavior, no gpu driver override
		{
			shared_ptr<GPUDriverD3D11> impl = make_shared<GPUDriverD3D11>();
			impl->RegisterOldReservedIds(oldEntries);
			m_GPUDriver->SetGPUImpl(impl);
		}
		else
		{
			ulOverrides.GPUDriver->RegisterOldReservedIds(oldEntries);
			m_GPUDriver->SetGPUImpl(ulOverrides.GPUDriver);
		}

		return true;
	}

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

	if (ulOverrides.FileSystem == nullptr)
	{
		m_FileSystem = std::make_shared<FileSystemWin>((DirectoryHelper::GetWebDirectory()).c_str());
	}
	else
	{
		m_FileSystem = ulOverrides.FileSystem;
	}
	ul::Platform::instance().set_file_system(m_FileSystem.get());


	m_GPUDriver = std::make_shared<RetargetableGPUDriverD3D11>();

	if (ulOverrides.GPUDriver == nullptr) //Default behavior, no gpu driver override
	{
		shared_ptr<GPUDriverD3D11> impl = make_shared<GPUDriverD3D11>();
		m_GPUDriver->SetGPUImpl(impl);
	}
	else
	{
		m_GPUDriver->SetGPUImpl(ulOverrides.GPUDriver);
	}
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
	m_WindowIdToViewIdMap.clear();
	m_ViewsMap.clear();
	m_WindowIdToWindowPtrMap.clear();
}

void UltralightManager::UpdateViews()
{
	//LOGINFO("Ultralight->Update()");
	m_UltralightRenderer->Update();
	//LOGINFO("Ultralight->Render()");
	m_UltralightRenderer->Render();
	//LOGINFO("GPUDriver->DrawCommandList() Start");
	m_GPUDriver->DrawCommandList();
	//LOGINFO("GPUDriver->DrawCommandList() Finished");
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
	if (parms.InspectionTarget != nullptr)
	{
		if (parms.InspectionTarget->IsInspectorView())
		{
			ErrorHandler::LogCriticalError("Attempted to create inspector view over another inspector view.");
			return nullptr;
		}
		if (parms.InspectionTarget->HasInspectorView())
		{
			ErrorHandler::LogCriticalError("Attempted to create inspector view for view that already contains an inspector view.");
			return nullptr;
		}
	}
	std::shared_ptr<UltralightView> pView = std::make_shared<UltralightView>();
	if (!pView->Initialize(parms))
	{
		ErrorHandler::LogCriticalError("Failed to create Ultralight View.");
		return nullptr;
	}

	if (parms.InspectionTarget != nullptr)
	{
		parms.InspectionTarget->m_InspectorView = pView;
		parms.InspectionTarget->GetView()->CreateLocalInspectorView();
	}

	m_ViewsMap.insert(std::make_pair(pView->GetId(), pView));
	return pView;
}

void UltralightManager::DestroyView(shared_ptr<UltralightView> pView)
{
	//Need to remove all references to this view
	int32_t windowId = pView->GetWindowId();
	if (pView->IsInspectorView())
	{
		if (pView->m_InspectionTarget->m_InspectorView == pView) //This should always be true
		{
			pView->m_InspectionTarget->m_InspectorView = nullptr;
		}
		else
		{
			DebugBreak();
			FatalError("Hmm.. DestroyView called for an inspector view but the inspection target did not reference it as the inspector view?");
		}
	}
	if (windowId != -1)
	{
		auto windowPair = m_WindowIdToViewIdMap.find(windowId);
		if (windowPair == m_WindowIdToViewIdMap.end())
		{
			FatalError("Failed to find window entry in window map.");
		}
		windowPair->second.erase(pView->GetId());
	}
	m_ViewsMap.erase(pView->GetId());
}

UltralightManager* UltralightManager::GetInstance()
{
	if (s_Instance == nullptr)
	{
		return GetInstanceShared().get();
	}
	return s_Instance.get();
}

shared_ptr<UltralightManager> UltralightManager::GetInstanceShared()
{
	if (s_Instance == nullptr)
	{
		//Workaround to pass private constructor into make_shared from https://stackoverflow.com/a/25069711
		struct make_shared_enabler : public UltralightManager {};

		s_Instance = make_shared<make_shared_enabler>();
		//s_Instance = make_shared<UltralightManager>();
	}
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
	auto iter = m_ViewsMap.find(viewId);
	if (iter == m_ViewsMap.end())
	{
		return nullptr;
	}
	return iter->second;
}

IGPUDriverD3D11* UltralightManager::GetGPUDriver()
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
		//LOGINFO("Ultralight display refresh");
		m_UltralightRenderer->RefreshDisplay(view.second->GetView()->display_id());
	}
}

bool UltralightManager::FireKeyboardEvent(KeyboardEvent* keyboardEvent)
{
	int32_t windowId = keyboardEvent->GetWindowId();
	Engine* pEngine = Engine::GetInstance();
	Window* pWindow = pEngine->GetWindowFromId(keyboardEvent->GetWindowId());

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
	Engine* pEngine = Engine::GetInstance();
	Window* pWindow = pEngine->GetWindowFromId(mouseEvent->GetWindowId());

	if (mouseEvent->GetType() == MouseEvent::Type::MouseUp) //Mouse up events will always get redirected to the focused view (if one is focused)
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
	Engine* pEngine = Engine::GetInstance();
	Window* pWindow = pEngine->GetWindowFromId(scrollEvent->GetWindowId());

	if (pWindow->m_FocusedUltralightView != nullptr)
	{
		pWindow->m_FocusedUltralightView->FireScrollEvent(scrollEvent->ToUltralightScrollEvent());
		return true;
	}

	return false;
}

shared_ptr<UltralightView> UltralightManager::GetUltralightViewFromNativeViewPtr(ul::View* pNativeView)
{
	//TODO: Inefficient lookup, maybe add another map for this lookup?
	for (auto mapPair : m_ViewsMap)
	{
		auto pView = mapPair.second;
		ul::View* pULView = pView->GetView();
		if (pULView == pNativeView)
		{
			return pView;
		}
	}
	ErrorHandler::LogCriticalError("UltralightManager::GetUltralightViewFromNativeViewPtr() failed.");
	return nullptr;
}

UltralightManager::~UltralightManager()
{
	m_UltralightRenderer = nullptr;
}

UltralightManager::UltralightManager()
{
	OutputDebugStringA("Ultralight Constructor!!");
}