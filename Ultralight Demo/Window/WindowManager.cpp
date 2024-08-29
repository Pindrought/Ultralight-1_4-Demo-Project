#include "PCH.h"
#include "WindowManager.h"
#include "../UltralightImpl/UltralightManager.h"
#include "../Engine.h"

map<int32_t, shared_ptr<Window>> WindowManager::s_WindowIdToWindowPtrMap;

weak_ptr<Window> WindowManager::SpawnWindow(const WindowCreationParameters& parms)
{
	std::shared_ptr<Window> pWindow = std::make_shared<Window>();
	if (!pWindow->Initialize(parms))
	{
		return weak_ptr<Window>();
	}

	s_WindowIdToWindowPtrMap[pWindow->GetId()] = pWindow;
	UltralightManager* pUltralightMgr = UltralightManager::GetInstance();
	pUltralightMgr->RegisterWindow(pWindow);
	return weak_ptr<Window>(pWindow);
}

void WindowManager::DestroyWindow(int32_t id)
{
	Engine* pEngine = Engine::GetInstance();
	assert(pEngine != nullptr);
	UltralightManager* pUltralightManager = UltralightManager::GetInstance();
	assert(pUltralightManager != nullptr);

	auto iter = s_WindowIdToWindowPtrMap.find(id);
	if (iter == s_WindowIdToWindowPtrMap.end())
	{
		return;
	}
	if (iter->second->m_DestructionInitiated)
	{
		return;
	}
	iter->second->m_DestructionInitiated = true; //This is to prevent errors from people calling DestroyWindow multiple times
	string msg = strfmt("WindowManager::DestroyWindow(%d)", id);
	LOGINFO(msg.c_str());

	pEngine->OnWindowDestroyStartCallback(id);
	pEngine->GetInputController()->ClearEventsForWindow(id);

	::DestroyWindow(s_WindowIdToWindowPtrMap[id]->GetHWND());
	msg = strfmt("WindowManager::DestroyWindow --> Removing From Map (%d)", id);
	LOGINFO(msg.c_str());
	s_WindowIdToWindowPtrMap.erase(id);

	pUltralightManager->RemoveWindowId(id);
	pEngine->OnWindowDestroyEndCallback(id);
}

void WindowManager::DestroyWindow(WeakWrapper<Window> window)
{	
	if (!window.expired())
	{
		DestroyWindow(window->GetId());
	}
}

void WindowManager::DestroyAllWindows()
{
	vector<int> windowIds;
	for (auto iter : s_WindowIdToWindowPtrMap)
	{
		windowIds.push_back(iter.first);
	}
	for (auto id : windowIds)
	{
		DestroyWindow(id);
	}
}

weak_ptr<Window> WindowManager::GetWindow(int32_t id)
{
	auto iter = s_WindowIdToWindowPtrMap.find(id);
	if (iter != s_WindowIdToWindowPtrMap.end())
	{
		return iter->second;
	}
    return weak_ptr<Window>();
}

int WindowManager::GetWindowCount()
{
	return s_WindowIdToWindowPtrMap.size();
}

const map<int32_t, shared_ptr<Window>>& WindowManager::GetWindowMap()
{
	return s_WindowIdToWindowPtrMap;
}
