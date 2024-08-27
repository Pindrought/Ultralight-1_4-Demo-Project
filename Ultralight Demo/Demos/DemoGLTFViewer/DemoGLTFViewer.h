#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "../Graphics/Renderable/Camera.h"
#include "../Graphics/Renderable/Entity.h"

class DemoGLTFViewer : public Engine
{
public:
	bool Startup() override;
	void OnPreRenderFrame() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	bool LoadModel(string filePath);
	shared_ptr<Camera> m_Camera = nullptr;
	shared_ptr<Scene> m_Scene = nullptr;
	shared_ptr<Model> m_LoadedModel = nullptr;
	shared_ptr<Entity> m_LoadedEntity = nullptr;

	shared_ptr<Window> m_PrimaryWindow = nullptr;
	shared_ptr<UltralightView> m_PrimaryView = nullptr;

	shared_ptr<UltralightView> m_OpenFileDialogView = nullptr;
	shared_ptr<Window> m_OpenFileDialogWindow = nullptr;
	string m_LastDirectoryAccessed = "";

};