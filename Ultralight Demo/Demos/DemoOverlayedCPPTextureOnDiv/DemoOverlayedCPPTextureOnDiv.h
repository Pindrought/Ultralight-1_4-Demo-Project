#pragma once
#include <PCH.h>
#include "../Engine.h"

//I would not suggest to ever use this method. It is just shown as a possible approach, but if possible should instead
//use the integrated texture method with a custom GPU driver. This method is not yet implemented into this project.
//I am waiting on an Ultralight update to make it possible.

class DemoOverlayedCPPTextureOnDiv : public Engine
{
public:
	bool Startup() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
	void OnPostRenderULViews() override; //This is where we draw our C++ texture into the render target

	WeakWrapper<Window> m_PrimaryWindow;
	struct DivRectData
	{
		float X = 0;
		float Y = 0;
		float Width = 0;
		float Height = 0;
	};
	DivRectData m_DivData;
	shared_ptr<Texture> m_CPPTexture = nullptr;
};