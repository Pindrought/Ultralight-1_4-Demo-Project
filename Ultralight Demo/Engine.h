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

	bool Initialize();
	virtual bool InitializeUltralight();
	bool IsRunning();
	void ProcessWindowsMessages();
	virtual bool Startup();
	virtual bool Tick(); //Tick is called once per frame before RenderFrame() and is where mouse/keyboard processing/redirection should occur.
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters) = 0;
	virtual void OnWindowDestroyStartCallback(int32_t windowId) = 0;
	virtual void OnWindowDestroyEndCallback(int32_t windowId) = 0;
	virtual void OnWindowResizeCallback(Window* pWindow) = 0;
	void SetRunning(bool running);
	std::shared_ptr<Window> SpawnWindow(const WindowCreationParameters& parms);
	bool CleanupWindow(int32_t windowId);
	Window* GetWindowFromId(int32_t windowId);
	void RenderFrame();
	~Engine();
protected:
	static Engine* s_Instance; //There will only ever be one engine instance
	InputController m_InputController; //Only one engine instance = only ever one input controller
	Renderer m_Renderer; //Only one engine instance = only ever one renderer
	unordered_map<int32_t, shared_ptr<Window>> m_WindowIdToWindowInstanceMap;
	shared_ptr<UltralightManager> m_UltralightMgr;
	bool m_IsRunning = false;
	bool m_VSync = true;
};