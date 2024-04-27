#pragma once
#include <PCH.h>
#include "Graphics/Renderer.h"
#include "Window/Window.h"
#include "Window/InputController/InputController.h"
#include "UltralightImpl/UltralightManager.h"

struct WindowDragInfo
{
	bool DragInProgress = false;
	std::shared_ptr<Window> pWindowBeingDragged = nullptr;
	POINT DragStartMousePosition;
	POINT DragStartWindowPosition;
};

class Engine
{
public:
	static Engine* GetInstance();
	InputController* GetInputController();

	bool Initialize(WindowCreationParameters windowParms);
	bool IsRunning();
	void ProcessWindowsMessages();
	void Tick();
	void SetRunning(bool running);
	std::shared_ptr<Window> SpawnWindow(const WindowCreationParameters& parms);
	bool CleanupWindow(int32_t windowId);
	Window* GetWindowFromId(int32_t windowId);
	EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyCallback(int32_t windowId);
private:
	void RenderFrame();
	static Engine* s_Instance; //There will only ever be one engine instance
	InputController m_InputController; //Only one engine instance = only ever one input controller
	Renderer m_Renderer; //Only one engine instance = only ever one renderer
	shared_ptr<Window> m_PrimaryWindow = nullptr;
	unordered_map<int32_t, shared_ptr<Window>> m_WindowIdToWindowInstanceMap;
	UltralightManager m_UltralightMgr;
	bool m_IsRunning = false;
	bool m_VSync = true;
	shared_ptr<UltralightView> m_UltralightTestView = nullptr;

	WindowDragInfo m_WindowDragInfo;
};