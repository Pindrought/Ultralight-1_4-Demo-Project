#pragma once
#include <PCH.h>
#include "WindowStyle.h"
#include "../Graphics/RenderTarget/RenderTargetContainer.h"
#include "WindowHelperForBorderlessResizable.h"

struct WindowCreationParameters
{
	int16_t Width = 768;
	int16_t Height = 1024;
	std::string Title = "Default Title";
	WindowStyle Style = WindowStyle::ExitButton;
	int XPosition = INT_MAX;
	int YPosition = INT_MAX;
	HWND ParentWindow = NULL;
};

class Window
{
	friend class UltralightManager;
	friend class WindowHelperForBorderlessResizable;
public:
	~Window();
	bool Initialize(const WindowCreationParameters& parms);
	int32_t GetId() const;
	HWND GetHWND() const;
	int32_t GetWidth() const;
	int32_t GetHeight() const;
	string GetTitle() const;
	WindowStyle GetStyle() const;
	bool ToggleClickthrough(bool clickthrough);
	void StartDrag();
	void StopDrag();
	IDXGISwapChain1* GetSwapChainPtr();
	RenderTargetContainer* GetRenderTargetContainer();
	LRESULT WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //This probably shouldn't be public since it should not be called outside of Window.cpp, but I don't have a great solution to hide this.
	const list<shared_ptr<UltralightView>>& GetSortedUltralightViews();
	bool IsWindowMaximized() const;
	void Maximize();
	void Restore();
	void Show();
	void Hide();
	void Close();
private:
	bool InitializeSwapchain();
	bool InitializeRenderTargetContainer();
	void RegisterWindowClass();
	bool ResizeSwapChainAndRenderTargetContainer();
	static int32_t GetAvailableWindowId();
	HWND m_HWND = NULL;
	string m_Title = "";
	int32_t m_Id = -1;
	int32_t m_Width = 0;
	int32_t m_Height = 0;
	WindowStyle m_Style = WindowStyle::None;
	string m_WindowClassName = "UltralightDemoWindow";

	bool m_IsMaximized = false;
	bool m_ClickThroughEnabled = false;
	bool m_DirectCompositionEnabled = false;
	ComPtr<IDCompositionTarget> m_DirectCompositionTarget = nullptr;
	ComPtr<IDCompositionVisual> m_DirectCompositionVisual = nullptr;
	
	bool m_IsBorderlessResizable = false; //If borderless resizable, there is lots of extra work
	BorderlessResizableWindowData m_BRWData;

	bool m_ResizeOrMoveInProgress = false;
	ComPtr<IDXGISwapChain1> m_SwapChain = nullptr;
	shared_ptr<RenderTargetContainer> m_RenderTargetContainer = nullptr;
	//TODO: Add z-index support
	list<shared_ptr<UltralightView>> m_UltralightViewsSorted; //Sorted based on z-index
	std::shared_ptr<UltralightView> m_FocusedUltralightView = nullptr;
};