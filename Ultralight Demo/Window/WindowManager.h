#pragma once
#include "PCH.h"
#include "Window.h"
class WindowManager
{
public:
	static weak_ptr<Window> SpawnWindow(const WindowCreationParameters& parms);
	static void DestroyWindow(int32_t id);
	static void DestroyWindow(WeakWrapper<Window> window);
	static void DestroyAllWindows();
	static weak_ptr<Window> GetWindow(int32_t id);
	static int GetWindowCount();
	static const map<int32_t, shared_ptr<Window>>& GetWindowMap();
private:
	static map<int32_t, shared_ptr<Window>> s_WindowIdToWindowPtrMap;
};