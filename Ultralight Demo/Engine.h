#pragma once
#include <PCH.h>
#include "Graphics/Renderer.h"
#include "Window/WindowManager.h"
#include "Window/InputController/InputController.h"
#include "UltralightImpl/UltralightManager.h"
#include "../Misc/Timer.h"

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
	virtual bool Startup() = 0;
	bool Tick();
	virtual bool TickStart() { return true;  };
	virtual bool ProcessInput();
	virtual bool TickEnd() { return true; };
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters) = 0;
	virtual void OnWindowDestroyStartCallback(int32_t windowId) = 0;
	virtual void OnWindowDestroyEndCallback(int32_t windowId) = 0;
	virtual void OnWindowResizeCallback(Window* pWindow) = 0;
	void SetRunning(bool running);
	virtual void OnPreRenderFrame() {};
	void RenderFrame();
	virtual void OnPreRenderULViews();
	virtual void OnPostRenderULViews();
	~Engine();
protected:
	static Engine* s_Instance; //There will only ever be one engine instance
	InputController m_InputController; //Only one engine instance = only ever one input controller
	Renderer m_Renderer; //Only one engine instance = only ever one renderer
	shared_ptr<UltralightManager> m_UltralightMgr;
	bool m_IsRunning = false;
	bool m_VSync = true;
	float m_DeltaTime = 0; //Delta time between each frame - calculated at the start of each tick
	vector<shared_ptr<RenderTargetContainer>> m_OffScreenRenderTargetContainers;
private:
	Timer m_FrameTimer;
};