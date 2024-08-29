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
	bool UpdateAnimationListForLoadedModel();
	shared_ptr<Camera> m_Camera = nullptr;
	shared_ptr<Scene> m_Scene = nullptr;
	shared_ptr<Model> m_LoadedModel = nullptr;
	shared_ptr<Entity> m_LoadedEntity = nullptr;

	WeakWrapper<Window> m_PrimaryWindow;
	WeakWrapper<UltralightView> m_PrimaryView;

	WeakWrapper<Window> m_OpenFileDialogWindow;
	WeakWrapper<UltralightView> m_OpenFileDialogView;
	string m_LastDirectoryAccessed = "";
	float m_RotationSpeed = 1.0f;
	float m_AnimationSpeed = 1.0f;
};