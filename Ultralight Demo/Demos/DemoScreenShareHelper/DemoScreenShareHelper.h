#pragma once
#include <PCH.h>
#include "../Engine.h"
#include "ScreenCapper.h"
//WIP - This still needs to be cleaned up and changes made.
class DemoScreenShareHelper : public Engine
{
public:
	bool Startup() override;
	~DemoScreenShareHelper();
	virtual EZJSParm OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters);
	void OnWindowDestroyStartCallback(int32_t windowId) override;
	void OnWindowDestroyEndCallback(int32_t windowId) override;
	void OnWindowResizeCallback(Window* pWindow) override;
	void OnPostRenderULViews() override; //This is where we draw our C++ texture into the render target
	void StopSharing();
	virtual void OnPreRenderULViews() override;

	bool OnMouseMove(int x, int y); //return value = skip passing down input event
	bool OnMouseDown(int x, int y); //return value = skip passing down input event
	bool OnMouseUp(int x, int y); //return value = skip passing down input event
	void RemoveHook();

private:
	void UpdateRectOutlineWindow();
	void RebuildRectMesh(int width, int height);
	WeakWrapper<Window> m_Window;
	WeakWrapper<UltralightView> m_View;
	WeakWrapper<Window> m_SelectedMonitorWindow;
	WeakWrapper<Window> m_PresentationWindow;
	WeakWrapper<Window> m_OutlineWindow;
	shared_ptr<Texture> m_PresentationTexture = nullptr;

	struct RectSelectionData {
		bool RectSelectionInProgress = false;
		HMONITOR HandleToMonitor = nullptr;
		int X1 = 0;
		int Y1 = 0;
		int X2 = 0;
		int Y2 = 0;
	};

	struct DragData {
		bool DragInProgress = false;
		int StartMouseX = 0;
		int StartMouseY = 0;
		int StartWindowX = 0;
		int StartWindowY = 0;
	};
	enum Corner
	{
		TOPLEFT,
		TOPRIGHT,
		BOTTOMLEFT,
		BOTTOMRIGHT,
		INVALID
	};

	struct ResizeData
	{
		bool ResizeInProgress = false;
		Corner Corner = Corner::INVALID;
		int MouseStartX = 0;
		int MouseStartY = 0;
		int WindowStartX = 0;
		int WindowStartY = 0;
		int WindowStartWidth = 0;
		int WindowStartHeight = 0;
	};

	ResizeData m_ResizingData;

	DragData m_DragData;

	RectSelectionData m_RectSelectionData;

	shared_ptr<Texture> m_CPPTexture = nullptr;
	bool SetHook();
	static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	static HHOOK s_HandleMouseHook;
	ScreenCapper m_ScreenCapper;
	const float c_TimeBetweenScreenUpdates = 100; //100ms
	float m_TimeSinceLastUpdate = 0;
	Mesh2DRenderData m_BorderRectMesh;
	const float m_OutlineThickness = 3.0f;
};