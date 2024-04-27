#pragma once
#include <PCH.h>
#include "WindowStyle.h"
#include "../Graphics/RenderTarget/RenderTargetContainer.h"

struct WindowCreationParameters
{
	int16_t Width = 768;
	int16_t Height = 1024;
	std::string Title = "Default Title";
	WindowStyle Style = WindowStyle::ExitButton;
	int XPosition = INT_MAX;
	int YPosition = INT_MAX;
	float WindowAlpha = 1.0f; //Must use WindowStyle::TransparencyAllowed
};

class Window
{
	friend class UltralightManager;
public:
	~Window();
	bool Initialize(const WindowCreationParameters& parms);
	int32_t GetId() const;
	HWND GetHWND() const;
	int32_t GetWidth() const;
	int32_t GetHeight() const;
	string GetTitle() const;
	WindowStyle GetStyle() const;
	BYTE GetAlpha() const;
	bool ToggleClickthrough(bool clickthrough);
	bool SetWindowAlpha(float alpha);
	bool SetWindowColorKey(COLORREF colorKey); //At one point this worked, but now I just have issues with it.
	IDXGISwapChain1* GetSwapChainPtr();
	RenderTargetContainer* GetRenderTargetContainer();
	LRESULT WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //This probably shouldn't be public since it should not be called outside of Window.cpp, but I don't have a great solution to hide this.
	const list<shared_ptr<UltralightView>>& GetSortedUltralightViews();
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

	bool m_TransparencyAllowed = true;
	bool m_ClickThroughEnabled = false;
	COLORREF m_ColorRef = RGB(0, 0, 0);
	bool m_ColorRefUsed = false; //Color Ref is for assigning a specific color to be fully transparent.
	BYTE m_Alpha = 255;
	bool m_TransparencyUsed = false;
	
	bool m_ResizeOrMoveInProgress = false;
	ComPtr<IDXGISwapChain1> m_SwapChain = nullptr;
	shared_ptr<RenderTargetContainer> m_RenderTargetContainer = nullptr;
	//TODO: Add z-index support
	list<shared_ptr<UltralightView>> m_UltralightViewsSorted; //Sorted based on z-index
	std::shared_ptr<UltralightView> m_FocusedUltralightView = nullptr;
};