#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoBasic : public Engine
{
public:
	bool Startup() override;
	bool Tick() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
};