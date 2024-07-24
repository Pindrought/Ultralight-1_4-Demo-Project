#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "../CustomGPUImpl/CustomGPUImpl.h"
#include "CustomFileSystem/CustomFileSystem.h"

class DemoCPPTextureInBrowser : public Engine
{
public:
	bool InitializeUltralight() override;
	bool Startup() override;
	bool Tick() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	map<string, shared_ptr<Texture>> m_CPPTexturesForBrowserMap;
	shared_ptr<CustomGPUImpl> m_CustomGPUImpl = nullptr;
	shared_ptr<CustomFileSystem> m_FileSystemImpl = nullptr;
};