#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoBorderlessResizable : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	std::shared_ptr<Window> m_PrimaryWindow = nullptr;
};