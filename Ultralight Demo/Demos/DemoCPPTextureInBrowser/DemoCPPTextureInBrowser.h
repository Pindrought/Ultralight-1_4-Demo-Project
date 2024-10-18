#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "../Graphics/Renderable/Camera.h"
#include "../Graphics/Renderable/Entity.h"
#include "../Graphics/MeshGenerator.h"

class DemoCPPTextureInBrowser : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
	~DemoCPPTextureInBrowser();
private:
	shared_ptr<Texture> m_Texture = nullptr;
	WeakWrapper<Window> m_Window;
	WeakWrapper<UltralightView> m_View;
};