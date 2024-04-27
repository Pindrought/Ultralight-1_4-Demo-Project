#pragma once
#include <PCH.h>
#include "RenderTarget.h"

class RenderTargetContainer
{
public:
	bool InitializeWindowRenderTarget(Window* pWindow,
									  UINT sampleCount = 8);
	bool InitializeRenderTarget(UINT width, UINT height,
								std::shared_ptr<Texture> renderTargetTexture = nullptr,
								std::shared_ptr<Texture> depthBufferTexture = nullptr,
								UINT sampleCount = 1);
	ID3D11RenderTargetView* GetRenderTargetView();
	ID3D11RenderTargetView* const* GetRenderTargetViewAddressOf();
	ID3D11Resource* GetTextureResource();

	ID3D11DepthStencilView* GetDepthStencilView();
	bool IsActive();
	void SetActive(bool activeStatus);
	bool Resize(UINT width,
				UINT height);
	D3D11_VIEWPORT& GetViewport();
	void SetBackgroundColor(float r, float g, float b, float a);
	float* GetBackgroundColor();
	void ResolveIfNecessary();
	void ResetRenderTargetsForResize();
	void UpdateViewportDimensions(UINT width, UINT height);

	UINT GetWidth();
	UINT GetHeight();
	bool IsWindowRenderTarget();
	Window* GetWindowPtr(); //This is only valid for render targets assigned/created for a window surface

	bool IsReadyForRender();
	void AdvanceUpdateInterval(float ms);
	void ResetReadyForRender();
	void SetUpdateInterval(float ms);

	~RenderTargetContainer();
private:
	bool m_IsActive = true;
	bool m_IsWindowRenderTarget = false;
	std::shared_ptr<RenderTarget> m_RenderTarget = nullptr;
	std::shared_ptr<RenderTarget> m_MSAARenderTarget = nullptr;
	D3D11_VIEWPORT m_Viewport{ 0 };
	UINT m_Width = 0;
	UINT m_Height = 0;
	float m_BGColor[4] = { 0, 0, 0, 0 };
	DXGI_FORMAT m_TextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	Window* m_WindowPtr = nullptr;
	float m_UpdateInterval = 0; //0 = never delay update, otherwise interval is in miliseconds for how often to update
	float m_UpdateIntervalProgress = 0;
};

