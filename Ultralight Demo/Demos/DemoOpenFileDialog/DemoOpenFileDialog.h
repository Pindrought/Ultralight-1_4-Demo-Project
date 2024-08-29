#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoOpenFileDialog : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	WeakWrapper<UltralightView> m_PrimaryView;
	WeakWrapper<UltralightView> m_OpenFileDialogView;
	WeakWrapper<Window> m_PrimaryWindow;
	WeakWrapper<Window> m_OpenFileDialogWindow;
	string m_LastDirectoryAccessed = "";
};