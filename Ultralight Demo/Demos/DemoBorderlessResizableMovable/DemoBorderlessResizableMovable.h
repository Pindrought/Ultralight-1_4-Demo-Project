#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoBorderlessResizableMovable : public Engine
{
public:
	bool Startup() override;
	bool Tick() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;

	void OnWindowResizeCallback(Window* pWindow) override;

	struct WindowDragInfo
	{
		bool DragInProgress = false;
		shared_ptr<Window> pWindowBeingDragged = nullptr;
		POINT DragStartMousePosition;
		POINT DragStartWindowPosition;
	};
	WindowDragInfo m_WindowDragInfo;
};