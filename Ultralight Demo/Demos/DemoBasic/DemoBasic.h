#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoBasic : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	WeakWrapper<Window> m_WindowForGPUView;
	WeakWrapper<Window> m_WindowForCPUView;
	WeakWrapper<UltralightView> m_GPUView;
	WeakWrapper<UltralightView> m_CPUView;
};