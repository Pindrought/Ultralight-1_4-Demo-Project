#pragma once
#include <PCH.h>
#include "../Engine.h"

class DemoSelector : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
	enum DemoId {
		None,
		DemoAntiAliasTest,
		DemoBasic,
		DemoBorderlessResizable,
		DemoBorderlessResizableMovable,
		DemoCPPTextureInBrowser,
		DemoInspector,
		DemoJSCPPCommunication,
		DemoOpenFileDialog,
		DemoTransparent,
		DemoOverlayedCPPTexture
	};
	DemoId m_SelectedDemo = DemoId::None;
	shared_ptr<Window> m_PrimaryWindow = nullptr;
};