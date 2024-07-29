#pragma once
//Basic demo to demonstrate using the inspector.
#include <PCH.h>
#include "../Engine.h"

class DemoInspector : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	shared_ptr<Window> m_MainWindow = nullptr;
	shared_ptr<Window> m_InspectorWindow = nullptr;
};