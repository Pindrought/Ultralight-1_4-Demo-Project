#include "PCH.h"
#include "DemoMultipleCubesEmbedded.h"
#include "../Misc/CursorManager.h"
#include "../Graphics/MeshGenerator.h"

bool DemoMultipleCubesEmbedded::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo Cube Draw";
	WeakWrapper<Window> pWindow = WindowManager::SpawnWindow(windowParms);
	if (pWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Height = pWindow->GetHeight();
	parms.Width = pWindow->GetWidth();
	parms.IsAccelerated = true;
	parms.IsTransparent = true;
	parms.ForceMatchWindowDimensions = true;
	parms.SampleCount = 8;
	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/MultipleCubesEmbedded/MultipleCubesEmbedded.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), pWindow->GetId());

	for (int i = 0; i < 4; i++)
	{
		shared_ptr<RenderTargetContainer> rt = make_shared<RenderTargetContainer>();
		if (!rt->InitializeRenderTarget(200, 200, nullptr, nullptr, 4))
		{
			return false;
		}

		shared_ptr<Scene> scene = make_shared<Scene>();

		shared_ptr<Camera> camera = make_shared<Camera>();
		float aspectRatio = (float)(200) / (float)(200);
		camera->InitializeProjectionValues(aspectRatio, 0.01f, 1000.0f);
		camera->SetPosition({ 0, 0, 5 });
		camera->SetScene(scene);

		m_Scenes.push_back(scene);
		rt->SetCamera(camera);

		shared_ptr<Mesh> mesh;
		switch (i)
		{
		case 0: //Solid white
			mesh = MeshGenerator::GenerateCube(MeshGenerator::MeshGenerationOption::DEFAULT);
			break;
		case 1: //Wireframe
		{
			mesh = MeshGenerator::GenerateCube(MeshGenerator::MeshGenerationOption::DEFAULT);
			for (auto primRef : mesh->m_PrimitiveRefs)
			{
				primRef->m_Material->PipelineState = m_Renderer.GetPipelineState("3D WIREFRAME");
			}
			break;
		}
		case 2: //AI Bowser texture
			mesh = MeshGenerator::GenerateCube(MeshGenerator::MeshGenerationOption::TEXTUREDVERTICES);
			break;
		case 3: //Colored Verts
			mesh = MeshGenerator::GenerateCube(MeshGenerator::MeshGenerationOption::COLOREDVERTICES);
			break;
		}

		shared_ptr<Model> model = make_shared<Model>();
		model->Initialize();
		model->AddMesh(mesh);
		shared_ptr<Entity> entity = make_shared<Entity>();
		entity->SetModel(model);
		scene->Entities.push_back(entity);
		m_Entities.push_back(entity);
		{ //Register this texture for embedding into image source
			IGPUDriverD3D11* pGPU = m_UltralightMgr->GetGPUDriver();
			uint32_t reservedTextureId = pGPU->RegisterCustomTextureAndReserveId(rt->GetTextureSharedPtr());
			ul::Rect bounds;
			bounds.left = 0.0f;
			bounds.top = 0.0f;
			bounds.right = 1.0f;
			bounds.bottom = 1.0f;
			ul::RefPtr<ul::ImageSource> imgSource = ul::ImageSource::CreateFromTexture(rt->GetWidth(),
																					   rt->GetHeight(),
																					   reservedTextureId,
																					   bounds);

			ul::ImageSourceProvider& provider = ul::ImageSourceProvider::instance();
			string id = strfmt("CUBE_%d", i);
			provider.AddImageSource(id.c_str(), imgSource);
			m_ImageSources.push_back(imgSource);
		}

		m_OffScreenRenderTargetContainers.push_back(rt);
	}

	return true;
}

void DemoMultipleCubesEmbedded::OnPreRenderFrame()
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
	for (auto& entity : m_Entities)
	{
		entity->SetRotation(q);
	}
	for (auto& imgSrc : m_ImageSources)
	{
		imgSrc->Invalidate();
	}
}

EZJSParm DemoMultipleCubesEmbedded::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	return EZJSParm();
}

void DemoMultipleCubesEmbedded::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	pWindow->DestroyAllViewsLinkedToThisWindow();
}

void DemoMultipleCubesEmbedded::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoMultipleCubesEmbedded::OnWindowResizeCallback(Window* pWindow)
{
}

void DemoMultipleCubesEmbedded::OnShutdown()
{
	ul::ImageSourceProvider& provider = ul::ImageSourceProvider::instance();
	m_ImageSources.clear();
	for (int i = 0; i < 4; i++)
	{
		string id = strfmt("CUBE_%d", i);
		provider.RemoveImageSource(id.c_str());
	}
}

shared_ptr<Entity> DemoMultipleCubesEmbedded::GenerateCubeEntity(MeshGenerator::MeshGenerationOption genOption, bool wireFrame)
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

