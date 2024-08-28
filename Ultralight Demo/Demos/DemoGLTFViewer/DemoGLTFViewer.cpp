#include "PCH.h"
#include "DemoGLTFViewer.h"
#include "../Misc/CursorManager.h"
#include "../Graphics/MeshGenerator.h"
#include <ShlObj.h>
#pragma comment(lib, "shell32.lib")
namespace fs = std::filesystem;

bool DemoGLTFViewer::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 600;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Demo GLTF Viewer";
	m_PrimaryWindow = SpawnWindow(windowParms);
	if (m_PrimaryWindow == nullptr)
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Height = m_PrimaryWindow->GetHeight();
	parms.Width = m_PrimaryWindow->GetWidth();
	parms.IsAccelerated = true;
	parms.IsTransparent = true;
	parms.SampleCount = 8;
	parms.ForceMatchWindowDimensions = true;
	m_PrimaryView = m_UltralightMgr->CreateUltralightView(parms);
	m_PrimaryView->LoadURL("file:///Samples/GLTFViewer/GLTFViewer.html");
	m_UltralightMgr->SetViewToWindow(m_PrimaryView->GetId(), m_PrimaryWindow->GetId());

	m_Scene = make_shared<Scene>();

	m_Camera = make_shared<Camera>();
	float aspectRatio = (float)(windowParms.Width) / (float)(windowParms.Height);
	m_Camera->InitializeProjectionValues(aspectRatio, 0.01f, 1000.0f);
	m_Camera->SetPosition({ 0, 0, 5 });
	m_Camera->SetScene(m_Scene);

	RenderTargetContainer* pRenderTarget = m_PrimaryWindow->GetRenderTargetContainer();
	assert(pRenderTarget != nullptr);
	pRenderTarget->SetCamera(m_Camera);

	m_LoadedEntity = make_shared<Entity>();
	m_Scene->Entities.push_back(m_LoadedEntity);

	string path = DirectoryHelper::GetAssetsDirectoryA() + "GLTF/cottage_blender/cottage_blender.gltf";
	if (!LoadModel(path))
	{
		string msg = strfmt("Failed to load model [%s].", path.c_str());
		MessageBoxA(m_PrimaryWindow->GetHWND(), msg.c_str(), msg.c_str(), MB_OK);
	}

	m_LastDirectoryAccessed = DirectoryHelper::GetAssetsDirectoryA() + "GLTF";

	return true;
}

void DemoGLTFViewer::OnPreRenderFrame()
{
	if (m_LoadedModel == nullptr)
	{
		return;
	}
	static float yaw = 0;
	//I want it to turn 1 time per second
	//1 full rotation = 2PI
	//m_DeltaTime = time between frames in miliseconds
	float secondsPerRotation = 5;
	float turnAmount = m_RotationSpeed * DirectX::XM_2PI * (m_DeltaTime / 1000.0f) / secondsPerRotation;
	yaw += turnAmount;
	while (yaw > DirectX::XM_2PI) //Not really necessary, but the further we get from 0 the less accurate so trying to keep it near 0
	{
		yaw -= DirectX::XM_2PI;
	}
	Quaternion q = Quaternion::CreateFromYawPitchRoll(yaw, 0, 0);
	m_LoadedEntity->SetRotation(q);
}

EZJSParm DemoGLTFViewer::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "OpenFileDialogLoaded")
	{
		shared_ptr<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);

		auto AddQuickAccessPath = [](UltralightView* pView, string displayPath, int directoryID, string appendedPath = "")
			{
				CHAR directoryPathLong[MAX_PATH];

				HRESULT resultFolderPath = SHGetFolderPathA(NULL, directoryID, NULL, SHGFP_TYPE_CURRENT, directoryPathLong);
				if (FAILED(resultFolderPath))
				{
					return;
				}

				std::string pathLongString(directoryPathLong);
				pathLongString += appendedPath;

				EZJSParm outReturnVal;
				string outException;
				bool result = pView->CallJSFnc("AddQuickAccessLocation",
											   { displayPath, pathLongString },
											   outReturnVal,
											   outException);
				if (result == false)
				{
					ErrorHandler::LogCriticalError("Failed to add quick access path to open file dialog.");
				}
			};

		for (auto& drive : DirectoryHelper::GetListOfDrives())
		{
			EZJSParm outReturnVal;
			string outException;
			bool result = pView->CallJSFnc("AddQuickAccessLocation",
										   { drive, drive },
										   outReturnVal,
										   outException);
			if (result == false)
			{
				ErrorHandler::LogCriticalError("Failed to add quick access path for drive to open file dialog.");
			}
		}

		AddQuickAccessPath(pView.get(), "User", CSIDL_PROFILE);
		AddQuickAccessPath(pView.get(), "Downloads", CSIDL_PROFILE, "\\Downloads");
		AddQuickAccessPath(pView.get(), "My Documents", CSIDL_PERSONAL);
		AddQuickAccessPath(pView.get(), "My Music", CSIDL_MYMUSIC);
		AddQuickAccessPath(pView.get(), "My Videos", CSIDL_MYVIDEO);
		AddQuickAccessPath(pView.get(), "Desktop", CSIDL_DESKTOP);

		{
			EZJSParm outReturnValue;
			string outException;
			pView->CallJSFnc("SetFileTypeFilter", { ".GLTF" }, outReturnValue, outException);
			pView->CallJSFnc("SetCurrentDirectory", { m_LastDirectoryAccessed }, outReturnValue, outException);
		}

		return EZJSParm();
	}
	if (eventName == "OpenFileDialog_OpenFolder")
	{
		if (parameters.size() == 1)
		{
			if (parameters[0].GetType() == EZJSParm::String)
			{
				try
				{
					string path = parameters[0].AsString();
					vector<EZJSParm> directoryEntries_Files;
					vector<EZJSParm> directoryEntries_Subdirectories;
					if (fs::is_directory(path) == false) {
						return false;
					}

					for (const auto& entry : fs::directory_iterator(path))
					{
						string path_utf8((char*)entry.path().generic_u8string().c_str()); //Idk if this is even valid, but it seems to be working?
						if (fs::is_directory(entry.path()))
						{
							directoryEntries_Subdirectories.push_back(path_utf8);
						}
						else
						{
							directoryEntries_Files.push_back(path_utf8);
						}
					}

					shared_ptr<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);
					EZJSParm outReturnVal;
					string outException;
					bool result = pView->CallJSFnc("UpdateDirectoryLocationAndEntries",
												   { path, directoryEntries_Subdirectories, directoryEntries_Files },
												   outReturnVal,
												   outException);
					if (result == false)
					{
						ErrorHandler::LogCriticalError("Failed to update directory location and entries.");
					}

					m_LastDirectoryAccessed = path;
					return true;
				}
				catch (std::exception ex)
				{
					ErrorHandler::LogCriticalError(ex.what());
					return false;
				}
			}
		}
		return false;
	}

	if (eventName == "OpenFileDialog_FilePicked")
	{
		if (parameters.size() == 1)
		{
			if (parameters[0].GetType() == EZJSParm::String)
			{
				string filePath = parameters[0].AsString();

				if (!LoadModel(filePath))
				{
					string msg = strfmt("Failed to load model [%s].", filePath.c_str());
					MessageBoxA(m_PrimaryWindow->GetHWND(), msg.c_str(), msg.c_str(), MB_OK);
					return false;
				}

				m_OpenFileDialogWindow->Close();



				EZJSParm outReturnValue;
				string outException;
				bool result = m_PrimaryView->CallJSFnc("UpdatePickedFilePath",
													   { filePath },
													   outReturnValue,
													   outException);
				if (result == false)
				{
					ErrorHandler::LogCriticalError("Issue dispatching picked file to primary view from open file dialog.");
				}

				UpdateAnimationListForLoadedModel(); //Builds & sends anim list to GLTFView Ultralight View
			}
		}
		return EZJSParm();
	}

	if (eventName == "GLTFViewer_Loaded")
	{
		UpdateAnimationListForLoadedModel();
		return EZJSParm();
	}

	if (eventName == "GLTFViewer_PlayAnimation")
	{
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Type::String);
		string animation = parameters[0].AsString();

		for (auto& anim : m_LoadedModel->m_Animations)
		{
			if (anim.Name == animation)
			{
				m_LoadedEntity->SetAnimationClip(&anim);
				return true;
			}
		}
	}

	if (eventName == "GLTFViewer_UpdateAnimationSpeed")
	{
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Type::Number);
		m_AnimationSpeed = parameters[0].AsDouble();
		m_LoadedEntity->SetAnimationSpeed(m_AnimationSpeed);
		return true;
	}

	if (eventName == "GLTFViewer_UpdateRotationSpeed")
	{
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Type::Number);
		m_RotationSpeed = parameters[0].AsDouble();
		return true;
	}


	if (eventName == "GLTFViewer_OpenFileDialog")
	{
		if (m_OpenFileDialogWindow == nullptr)
		{
			int monitorWidth = GetSystemMetrics(SM_CXSCREEN);
			int monitorHeight = GetSystemMetrics(SM_CYSCREEN);

			WindowCreationParameters windowParms;
			windowParms.Width = monitorWidth * 3 / 4;
			windowParms.Height = monitorHeight * 3 / 4;
			windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
			windowParms.Title = "OpenFileDialog File Selection";
			windowParms.ParentWindow = m_PrimaryWindow->GetHWND(); //By setting the parent, this window will always be on top.
			shared_ptr<Window> pWindow = SpawnWindow(windowParms);
			m_OpenFileDialogWindow = pWindow;
			if (m_OpenFileDialogWindow == nullptr)
			{
				FatalError("Failed to initialize open file dialog window. Program must now abort.");
			}
			m_PrimaryWindow->Disable();
		}
		if (m_OpenFileDialogView == nullptr)
		{
			UltralightViewCreationParameters parms;
			parms.Width = m_OpenFileDialogWindow->GetWidth();
			parms.Height = m_OpenFileDialogWindow->GetHeight();
			parms.IsAccelerated = false;
			parms.ForceMatchWindowDimensions = true;
			parms.IsTransparent = true;

			m_OpenFileDialogView = m_UltralightMgr->CreateUltralightView(parms);
			m_OpenFileDialogView->LoadURL("file:///Samples/OpenFileDialog/OpenFileDialog.html");
		}
		m_UltralightMgr->SetViewToWindow(m_OpenFileDialogView->GetId(), m_OpenFileDialogWindow->GetId());
		m_OpenFileDialogWindow->Show();
		return EZJSParm();
	}

	return EZJSParm();
}

void DemoGLTFViewer::OnWindowDestroyStartCallback(int32_t windowId)
{
	Window* pWindow = GetWindowFromId(windowId);
	if (m_OpenFileDialogWindow != nullptr)
	{
		if (windowId == m_OpenFileDialogWindow->GetId())
		{
			m_OpenFileDialogWindow = nullptr;
			m_PrimaryWindow->Enable();
		}
	}
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		if (pView == m_OpenFileDialogView) //Technically this is inefficient. Could keep the view alive and just reassign it to window, but this is simpler to manage.
		{
			m_OpenFileDialogView = nullptr;
		}
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoGLTFViewer::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (m_WindowIdToWindowInstanceMap.size() == 0)
	{
		SetRunning(false);
	}
}

void DemoGLTFViewer::OnWindowResizeCallback(Window* pWindow)
{
}

bool DemoGLTFViewer::LoadModel(string filePath)
{

	shared_ptr<Model> model = make_shared<Model>();
	string errorMsg;
	if (!model->Initialize(filePath, errorMsg))
	{
		return false;
	}
	m_LoadedModel = model;
	m_LoadedEntity->SetModel(m_LoadedModel);
	m_LoadedEntity->SetAnimationSpeed(m_AnimationSpeed);

	//I want to reposition the camera to ensure the model fits
	//I need to calculate a bounding sphere
	auto& aabb = m_LoadedModel->m_AABB;

	float radius = aabb.GetRadius();
	float camHeight = 0;// (cubeModel->m_AABB.MaxCoord.y - cubeModel->m_AABB.MinCoord.y) / 2 + cubeModel->m_AABB.MinCoord.y;
	//Algorithm to determine suggested distance from camera
	//https://stackoverflow.com/questions/21544336/how-to-position-the-camera-so-that-my-main-object-is-entirely-visible-and-fit-to
	float d1 = radius / sin(m_Camera->GetHorizontalFOV() / 2);
	float d2 = radius / sin(m_Camera->GetVerticalFOV() / 2);
	//Multiplying the distance by 2 because it's possible for model to go off screen when rotating if it's not centered.
	float distance = std::max(d1, d2) * 2;
	//
	DirectX::XMFLOAT3 camPos = aabb.GetCenter();
	m_Camera->SetPosition({ camPos.x, camPos.y, distance });

	return true;
}

bool DemoGLTFViewer::UpdateAnimationListForLoadedModel()
{
	//I need to update the view with the animations for this new model.
	EZJSParm outReturnValue;
	string outException;
	vector<EZJSParm> animationEntries;
	for (auto& animation : m_LoadedModel->m_Animations)
	{
		animationEntries.push_back(animation.Name);
	}
	EZJSParm animationList(animationEntries);

	bool result = m_PrimaryView->CallJSFnc("UpdateAnimationList",
											{ animationList },
											outReturnValue,
											outException);
	if (result == false)
	{
		ErrorHandler::LogCriticalError("Issue updating animation list.");
		return false;
	}
	return true;
}
