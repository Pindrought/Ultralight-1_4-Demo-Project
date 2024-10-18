#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoAntiAliasTest : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	vector<WeakWrapper<Window>> m_Windows;
	vector<WeakWrapper<UltralightView>> m_Views;
};