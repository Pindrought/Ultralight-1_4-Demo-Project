#include "PCH.h"
#include "DemoCubeDraw.h"
#include "../Misc/CursorManager.h"
#include "../Graphics/MeshGenerator.h"

bool DemoCubeDraw::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo Cube Draw";
	shared_ptr<Window> pWindow = SpawnWindow(windowParms);
	if (pWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Height = 200;
	parms.Width = 300;
	parms.IsAccelerated = true;
	parms.IsTransparent = true;
	parms.SampleCount = 8;
	shared_ptr<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/CubeDraw/CubeDraw.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());


	m_Scene = make_shared<Scene>();

	m_Camera = make_shared<Camera>();
	float aspectRatio = (float)(windowParms.Width) / (float)(windowParms.Height);
	m_Camera->InitializeProjectionValues(aspectRatio, 0.01f, 1000.0f);
	m_Camera->SetPosition({ 0, 0, 5 });
	m_Camera->SetScene(m_Scene);
	
	RenderTargetContainer* pRenderTarget = pWindow->GetRenderTargetContainer();
	assert(pRenderTarget != nullptr);
	pRenderTarget->SetCamera(m_Camera);

	m_CubeEntity = GenerateCubeEntity();
	if (m_CubeEntity == nullptr)
	{
		return false;
	}

	m_Scene->Entities.push_back(m_CubeEntity);

	return true;
}

void DemoCubeDraw::OnPreRenderFrame()
{
	static float yaw = 0;
	//I want it to turn 1 time per second
	//1 full rotation = 2PI
	//m_DeltaTime = time between frames in miliseconds
	float secondsPerRotation = 5;
	float turnAmount = DirectX::XM_2PI * (m_DeltaTime / 1000.0f) / secondsPerRotation;
	yaw += turnAmount;
	while (yaw > DirectX::XM_2PI) //Not really necessary, but the further we get from 0 the less accurate so trying to keep it near 0
	{
		yaw -= DirectX::XM_2PI;
	}
	Quaternion q = Quaternion::CreateFromYawPitchRoll(yaw, 0, 0);
	m_CubeEntity->SetRotation(q);
}

EZJSParm DemoCubeDraw::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "CubeMeshOptionsUpdate")
	{
		assert(parameters.size() == 3);
		assert(parameters[0].GetType() == EZJSParm::Type::Boolean);
		assert(parameters[1].GetType() == EZJSParm::Type::Boolean);
		assert(parameters[2].GetType() == EZJSParm::Type::Boolean);

		bool wireframe = parameters[0].AsBool();
		bool coloredVertices = parameters[1].AsBool();
		bool texturedVertices = parameters[2].AsBool();

		MeshGenerator::MeshGenerationOption option = MeshGenerator::MeshGenerationOption::DEFAULT;
		if (coloredVertices) {
			option = MeshGenerator::MeshGenerationOption::COLOREDVERTICES;
		}
		if (texturedVertices) {
			option = MeshGenerator::MeshGenerationOption::TEXTUREDVERTICES;
		}

		shared_ptr<Entity> entity = GenerateCubeEntity(option, wireframe);
		if (entity == nullptr)
		{
			return false;
		}

		m_CubeEntity = entity;

		m_Scene->Entities.clear();
		m_Scene->Entities.push_back(m_CubeEntity);

		return true;
	}
	return EZJSParm();
}

void DemoCubeDraw::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoCubeDraw::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoCubeDraw::OnWindowResizeCallback(Window* pWindow)
{
}

shared_ptr<Entity> DemoCubeDraw::GenerateCubeEntity(MeshGenerator::MeshGenerationOption genOption, bool wireFrame)
{
	shared_ptr<Mesh> cubeMesh = MeshGenerator::GenerateCube(genOption);

	if (cubeMesh == nullptr)
	{
		return nullptr;
	}

	if (wireFrame)
	{
		for (auto primRef : cubeMesh->m_PrimitiveRefs)
		{
			primRef->m_Material->PipelineState = m_Renderer.GetPipelineState("3D WIREFRAME");
		}
	}

	shared_ptr<Model> cubeModel = make_shared<Model>();
	cubeModel->Initialize();
	cubeModel->AddMesh(cubeMesh);
	shared_ptr<Entity> cubeEntity = make_shared<Entity>();
	cubeEntity->SetModel(cubeModel);
	return cubeEntity;
}

