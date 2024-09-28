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

	m_GPUDriver = make_shared<GPUDriverD3D11>();
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
	m_WeakViewsMap.clear();
	m_WeakAcceleratedViewsMap.clear();
	m_OwnedViewsMap.clear();
	m_WindowIdToWindowPtrMap.clear();
}

void UltralightManager::UpdateViews()
{
	//LOGINFO("Ultralight->Update()");
	if (m_OwnedViewsMap.size() == 0)
	{
		//DebugBreak();
	}
	m_UltralightRenderer->Update();
	//LOGINFO("Ultralight->Render() --> Ultralight->DrawCommandList()");
	m_UltralightRenderer->Render();
	//LOGINFO("GPUDriver->DrawCommandList() Start");
	m_GPUDriver->DrawCommandList();
	//LOGINFO("GPUDriver->DrawCommandList() Finished");
}

ul::Renderer* UltralightManager::GetRendererPtr()
{
	return m_UltralightRenderer.get();
}

bool UltralightManager::IsViewFlaggedForDeletion(int32_t viewId)
{
	return m_UltralightViewIdReferencesFlaggedForDeletion.contains(viewId);
}

void UltralightManager::RemoveViewFromWindow(int32_t viewId, int32_t windowId)
{
	//TODO: Add error checking
	auto windowIter = m_WindowIdToViewIdMap.find(windowId);
	if (windowIter != m_WindowIdToViewIdMap.end())
	{
		windowIter->second.erase(viewId);
	}
	auto viewIter = m_OwnedViewsMap.find(viewId);
	if (viewIter != m_OwnedViewsMap.end())
	{
		viewIter->second->SetToWindow(-1);
	}
	Engine* pEngine = Engine::GetInstance();
	assert(pEngine != nullptr);
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	assert(!pWindow.expired());
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
	auto viewIter = m_OwnedViewsMap.find(viewId);
	if (viewIter == m_OwnedViewsMap.end())
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
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	assert(!pWindow.expired());
	auto& viewList = pWindow->m_UltralightViewsSorted;
	bool viewInserted = false;
	for (auto listIter = viewList.begin(); listIter != viewList.end(); listIter++)
	{
		//CODENOTE[LOW] I am not sure why get() is necessary here, but I am too tired to worry about it right now.
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

void UltralightManager::RegisterWindow(WeakWrapper<Window> pWindow)
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

WeakWrapper<UltralightView> UltralightManager::CreateUltralightView(UltralightViewCreationParameters parms)
{
	if (parms.InspectionTarget != nullptr)
	{
		if (parms.InspectionTarget->IsInspectorView())
		{
			ErrorHandler::LogCriticalError("Attempted to create inspector view over another inspector view.");
			return WeakWrapper<UltralightView>();
		}
		if (parms.InspectionTarget->HasInspectorView())
		{
			ErrorHandler::LogCriticalError("Attempted to create inspector view for view that already contains an inspector view.");
			return WeakWrapper<UltralightView>();
		}
	}

	//Workaround to pass private constructor into make_shared from https://stackoverflow.com/a/25069711
	struct make_shared_enabler : public UltralightView {};
	shared_ptr<UltralightView> pView = make_shared<make_shared_enabler>();
	if (!pView->Initialize(parms))
	{
		ErrorHandler::LogCriticalError("Failed to create Ultralight View.");
		return WeakWrapper<UltralightView>();
	}

	int32_t id = pView->GetId();
	auto& refEntry = m_UltralightViewIdReferenceForLoadListener[id];
	refEntry = make_shared<int32_t>(id);
	pView->m_LoadListener->AssignViewId(refEntry);

	if (parms.InspectionTarget != nullptr)
	{
		parms.InspectionTarget->m_InspectorView = pView;
		parms.InspectionTarget->GetView()->CreateLocalInspectorView();
	}

	m_OwnedViewsMap.insert(std::make_pair(pView->GetId(), pView));
	m_WeakViewsMap.insert(make_pair(pView->GetId(), pView));
	if (parms.IsAccelerated)
	{
		m_WeakAcceleratedViewsMap.insert(std::make_pair(pView->GetId(), pView));
	}
	return pView;
}

void UltralightManager::DestroyView(WeakWrapper<UltralightView> pView)
{
	if (pView.expired()) //Has this already been destroyed? skip
		return;
	//CODENOTE[HIGH] - NEED TO REVIEW THE ORDER OF ALL OF THIS
	if (pView->m_DestructionInitiated)
	{
		return;
	}
	pView->m_DestructionInitiated = true;
	//Need to remove all references to this view
	int32_t windowId = pView->GetWindowId();
	if (pView->IsInspectorView())
	{
		if (pView->m_InspectionTarget->m_InspectorView == pView) //This should always be true
		{
			pView->m_InspectionTarget->m_InspectorView = WeakWrapper<UltralightView>();
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
		pView->SetToWindow(-1);
	}

	m_UltralightViewIdReferencesFlaggedForDeletion.insert(pView->GetId());
	m_WeakAcceleratedViewsMap.erase(pView->GetId());
	m_WeakViewsMap.erase(pView->GetId());
	m_OwnedViewsMap.erase(pView->GetId());

	//CODENOTE[LOW]: Find a way to avoid doing multiple updates when destroying multiple views
	LOGINFO("UltralightManager::DestroyView() m_UltralightRenderer->Update() start"); //Only doing the Update here to get outstanding window interval events out of the way
	m_UltralightRenderer->Update(); //Crash occurs here if a select dropdown was interacted with
	LOGINFO("UltralightManager::DestroyView() m_UltralightRenderer->Update() fin");

	for (auto id : m_UltralightViewIdReferencesFlaggedForDeletion)
	{
		m_UltralightViewIdReferenceForLoadListener.erase(id); //This is the location the int for the id the CallEvent function references for the view is stored - wait to delete it until definitely no more events
															  //Previously it was stored with the load listener, but that isn't possible anymore since load listener can be destructed while outstanding events exist
	}
	m_UltralightViewIdReferencesFlaggedForDeletion.clear();
}

void UltralightManager::DestroyAllViews()
{
	vector<WeakWrapper<UltralightView>> viewsToDestroy;
	for (auto iter : m_OwnedViewsMap)
	{
		viewsToDestroy.push_back(iter.second);
	}
	for (auto view : viewsToDestroy)
	{
		DestroyView(view);
	}
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

vector<WeakWrapper<UltralightView>> UltralightManager::GetViewsForWindow(int32_t windowId)
{
	//CODENOTE[LOW] Optimize this
	//TODO: Error checking and optimize these unnecessary lookups
	vector<WeakWrapper<UltralightView>> views;
	auto windowIter = m_WindowIdToViewIdMap.find(windowId);
	if (windowIter != m_WindowIdToViewIdMap.end())
	{
		for (auto viewId : windowIter->second)
		{
			auto viewIter = m_OwnedViewsMap.find(viewId);
			if (viewIter != m_OwnedViewsMap.end())
			{
				views.push_back(viewIter->second);
			}
		}
	}
	return views;
}

WeakWrapper<UltralightView> UltralightManager::GetViewFromId(int32_t viewId)
{
	//CODENOTE[LOW] Error checking?
	auto iter = m_OwnedViewsMap.find(viewId);
	if (iter == m_OwnedViewsMap.end())
	{
		return weak_ptr<UltralightView>();
	}
	return iter->second;
}

IGPUDriverD3D11* UltralightManager::GetGPUDriver()
{
	return m_GPUDriver.get();
}

unordered_map<int32_t, WeakWrapper<UltralightView>> UltralightManager::GetViews()
{
	return m_WeakViewsMap;
}

unordered_map<int32_t, WeakWrapper<UltralightView>> UltralightManager::GetAcceleratedViews()
{
	return m_WeakAcceleratedViewsMap;
}

void UltralightManager::RefreshViewDisplaysForAnimations()
{
	for (auto view : m_OwnedViewsMap)
	{
		//LOGINFO("Ultralight display refresh");
		m_UltralightRenderer->RefreshDisplay(view.second->GetView()->display_id());
	}
}

bool UltralightManager::FireKeyboardEvent(KeyboardEvent* keyboardEvent)
{
	int32_t windowId = keyboardEvent->GetWindowId();
	Engine* pEngine = Engine::GetInstance();
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(keyboardEvent->GetWindowId());

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

	/*string msg = strfmt("FireMouseEvent %s", mouseEvent->ToString().c_str());
	LOGINFO(msg.c_str());*/

	int32_t windowId = mouseEvent->GetWindowId();
	Engine* pEngine = Engine::GetInstance();
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(mouseEvent->GetWindowId());

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
		WeakWrapper<UltralightView> pView = *iter;
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
		if (result.x >= 0 && result.x < 1 &&
			result.y >= 0 && result.y < 1) //Was mouse event inside the rectangle of the html view?
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
		pWindow->m_FocusedUltralightView = WeakWrapper<UltralightView>();
	}
	return dispatchedToHtml;
}

bool UltralightManager::FireScrollEvent(ScrollEvent* scrollEvent)
{
	int32_t windowId = scrollEvent->GetWindowId();
	Engine* pEngine = Engine::GetInstance();
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(scrollEvent->GetWindowId());

	if (pWindow->m_FocusedUltralightView != nullptr)
	{
		pWindow->m_FocusedUltralightView->FireScrollEvent(scrollEvent->ToUltralightScrollEvent());
		return true;
	}

	return false;
}

WeakWrapper<UltralightView> UltralightManager::GetUltralightViewFromNativeViewPtr(ul::View* pNativeView)
{
	//TODO: Inefficient lookup, maybe add another map for this lookup?
	for (auto mapPair : m_WeakViewsMap)
	{
		auto pView = mapPair.second;
		ul::View* pULView = pView->GetView();
		if (pULView == pNativeView)
		{
			return pView;
		}
	}
	ErrorHandler::LogCriticalError("UltralightManager::GetUltralightViewFromNativeViewPtr() failed.");
	return WeakWrapper<UltralightView>();
}

UltralightManager::~UltralightManager()
{
	m_UltralightRenderer = nullptr;
}

UltralightManager::UltralightManager()
{
	OutputDebugStringA("Ultralight Constructor!!");
}