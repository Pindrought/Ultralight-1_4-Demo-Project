#pragma once
#include <PCH.h>
#include "../Texture.h"

class RenderTarget
{
	friend RenderTargetContainer;
public:
	ID3D11RenderTargetView* GetRTV();
	ID3D11RenderTargetView* const* GetRTVAddressOf();
	ID3D11DepthStencilView* GetDepthStencilView();
	ID3D11Resource* GetRTVTextureResourcePtr();
	Texture* GetDepthBufferTexturePtr();
	std::shared_ptr<Texture> GetSharedTexturePtr();
	DXGI_FORMAT GetTextureFormat();
	~RenderTarget();
private:
	RenderTarget() { /*This should only be constructed by RenderTargetContainer*/ }

	bool InitializeWindowRenderTarget(UINT width,
									  UINT height,
									  Window* pWindow,
									  std::shared_ptr<Texture> depthBufferTexture);
	bool Initialize(UINT width,
					UINT height,
					std::shared_ptr<Texture> renderTargetTexture = nullptr,
					std::shared_ptr<Texture> depthBufferTexture = nullptr,
					UINT sampleCount = 1,
					DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	bool RebuildTexture();
	bool RebuildRenderTargetView();
	bool RebuildDepthStencil();
	bool Resize(UINT width, UINT height);

	UINT m_Width = 0;
	UINT m_Height = 0;
	UINT m_SampleCount = 1;
	//Render Target View
	ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;
	//Render Target Texture Raw Ptr
	ComPtr<ID3D11Resource> m_TexturePtr = nullptr;
	//Render Target Texture [This will remain null for the primary render target since the swapchain generates backbuffer texture]
	std::shared_ptr<Texture> m_RenderTargetTexture = nullptr;
	//Depth Stencil buffer & view
	std::shared_ptr<Texture> m_DepthBufferTexture = nullptr;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView = nullptr;
	DXGI_FORMAT m_TextureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	Window* m_WindowPtr = nullptr; //If this render target is associated with a window, this will point to the window. Otherwise, nullptr
	bool m_IsWindowRenderTarget = false; //If this render target is associated with a specific window+swapchain
};