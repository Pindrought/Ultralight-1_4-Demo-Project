#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoOpenFileDialog : public Engine
{
public:
	bool Startup() override;
	bool Tick() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	shared_ptr<UltralightView> m_PrimaryView = nullptr;
	shared_ptr<UltralightView> m_OpenFileDialogView = nullptr;
	shared_ptr<Window> m_OpenFileDialogWindow = nullptr;
	string m_LastDirectoryAccessed = "";
};