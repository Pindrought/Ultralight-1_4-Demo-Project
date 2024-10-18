#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoBorderlessResizableMovable : public Engine
{
public:
	bool Startup() override;
	bool ProcessInput() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;

	void OnWindowResizeCallback(Window* pWindow) override;

	struct WindowDragInfo
	{
		bool DragInProgress = false;
		WeakWrapper<Window> pWindowBeingDragged;
		POINT DragStartMousePosition;
		POINT DragStartWindowPosition;
	};
	WindowDragInfo m_WindowDragInfo;
private:
	WeakWrapper<Window> m_Window;
	WeakWrapper<UltralightView> m_View;
};