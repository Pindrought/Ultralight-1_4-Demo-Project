#include "PCH.h"
#include "DemoOpenFileDialog.h"
#include "../Misc/CursorManager.h"
namespace fs = std::filesystem;

bool DemoOpenFileDialog::Startup()
{
	WindowCreationParameters windowParms;
	windowParms.Width = 800;
	windowParms.Height = 200;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "OpenFileDialog Demo Main Window";
	m_PrimaryWindow = WindowManager::SpawnWindow(windowParms);
	if (m_PrimaryWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_PrimaryWindow->GetWidth();
	parms.Height = m_PrimaryWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	m_PrimaryView = m_UltralightMgr->CreateUltralightView(parms);
	m_PrimaryView->LoadURL("file:///Samples/OpenFileDialog/Startup.html");
	m_UltralightMgr->SetViewToWindow(m_PrimaryView->GetId(), m_PrimaryWindow->GetId());

	m_LastDirectoryAccessed = DirectoryHelper::GetExecutableDirectoryA();

	return true;
}

EZJSParm DemoOpenFileDialog::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "OpenFileDialogLoaded")
	{
		WeakWrapper<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);
			
		auto AddQuickAccessPath = [](WeakWrapper<UltralightView> pView, string displayPath, int directoryID, string appendedPath = "")
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

		AddQuickAccessPath(pView, "User", CSIDL_PROFILE);
		AddQuickAccessPath(pView, "Downloads", CSIDL_PROFILE, "\\Downloads");
		AddQuickAccessPath(pView, "My Documents", CSIDL_PERSONAL);
		AddQuickAccessPath(pView, "My Music", CSIDL_MYMUSIC);
		AddQuickAccessPath(pView, "My Videos", CSIDL_MYVIDEO);
		AddQuickAccessPath(pView, "Desktop", CSIDL_DESKTOP);

		{
			EZJSParm outReturnValue;
			string outException;
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


					WeakWrapper<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);
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
			}
		}
		return EZJSParm();
	}
	
	if (eventName == "OpenFileDialog")
	{
		if (m_OpenFileDialogWindow.expired())
		{
			int monitorWidth = GetSystemMetrics(SM_CXSCREEN);
			int monitorHeight = GetSystemMetrics(SM_CYSCREEN);

			WindowCreationParameters windowParms;
			windowParms.Width = monitorWidth * 3 / 4;
			windowParms.Height = monitorHeight * 3 / 4;
			windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
			windowParms.Title = "OpenFileDialog File Selection";
			windowParms.ParentWindow = m_PrimaryWindow->GetHWND(); //By setting the parent, this window will always be on top.
			m_OpenFileDialogWindow = WindowManager::SpawnWindow(windowParms);
			if (m_OpenFileDialogWindow.expired())
			{
				FatalError("Failed to initialize open file dialog window. Program must now abort.");
			}
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

void DemoOpenFileDialog::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();

	vector<WeakWrapper<UltralightView>> viewsToDestroy;
	for (auto pView : pViews)
	{
		viewsToDestroy.push_back(pView);
	}
	for (auto view : viewsToDestroy)
	{
		m_UltralightMgr->DestroyView(view);
	}
}

void DemoOpenFileDialog::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoOpenFileDialog::OnWindowResizeCallback(Window* pWindow)
{
}

