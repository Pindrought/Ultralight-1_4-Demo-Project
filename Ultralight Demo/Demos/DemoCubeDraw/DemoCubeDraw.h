#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "../Graphics/Renderable/Camera.h"
#include "../Graphics/Renderable/Entity.h"
#include "../Graphics/MeshGenerator.h"

class DemoCubeDraw : public Engine
{
public:
	bool Startup() override;
	void OnPreRenderFrame() override;
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
private:
	shared_ptr<Entity> GenerateCubeEntity(MeshGenerator::MeshGenerationOption genOption = MeshGenerator::MeshGenerationOption::DEFAULT, bool wireFrame = false);
	shared_ptr<Camera> m_Camera = nullptr;
	shared_ptr<Scene> m_Scene = nullptr;
	shared_ptr<Entity> m_CubeEntity = nullptr;
};