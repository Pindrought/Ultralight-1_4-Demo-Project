#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "../Graphics/Renderable/Camera.h"
#include "../Graphics/Renderable/Entity.h"
#include "../Graphics/MeshGenerator.h"

class DemoMultipleCubesEmbedded : public Engine
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
	vector<shared_ptr<Scene>> m_Scenes;
	vector<shared_ptr<Entity>> m_Entities;
	vector<ul::RefPtr<ul::ImageSource>> m_ImageSources;
};