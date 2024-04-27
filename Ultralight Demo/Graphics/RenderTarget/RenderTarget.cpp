#include <PCH.h>
#include "RenderTarget.h"
#include "../../Engine.h"
#include "../../Window/Window.h"

ID3D11RenderTargetView* RenderTarget::GetRTV()
{
	return m_RenderTargetView.Get();
}

ID3D11RenderTargetView* const* RenderTarget::GetRTVAddressOf()
{
	return m_RenderTargetView.GetAddressOf();
}

ID3D11DepthStencilView* RenderTarget::GetDepthStencilView()
{
	return m_DepthStencilView.Get();
}


ID3D11Resource* RenderTarget::GetRTVTextureResourcePtr()
{
	return m_TexturePtr.Get();
}

DXGI_FORMAT RenderTarget::GetTextureFormat()
{
	return m_TextureFormat;
}

bool RenderTarget::InitializeWindowRenderTarget(UINT width,
												UINT height,
												Window* pWindow,
												std::shared_ptr<Texture> depthBufferTexture)
{
	m_Width = width;
	m_Height = height;
	m_WindowPtr = pWindow;
	m_IsWindowRenderTarget = true;

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	m_DepthBufferTexture = depthBufferTexture;
	if (m_DepthBufferTexture == nullptr)
	{
		m_DepthBufferTexture = std::make_shared<Texture>();
	}

	m_SampleCount = 1; //The primary back buffer will never have MSAA. There will be a separate render target that resolves to this when presenting if MSAA is "enabled".

	if (!Resize(m_Width, m_Height))
		return false;

	return true;
}

bool RenderTarget::Initialize(UINT width,
							  UINT height,
							  std::shared_ptr<Texture> renderTargetTexture,
							  std::shared_ptr<Texture> depthBufferTexture,
							  UINT sampleCount,
							  DXGI_FORMAT format)
{
	m_Width = width;
	m_Height = height;
	m_IsWindowRenderTarget = false;

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	m_TextureFormat = format;

	m_RenderTargetTexture = renderTargetTexture;
	if (m_RenderTargetTexture == nullptr)
	{
		m_RenderTargetTexture = std::make_shared<Texture>();
	}

	m_TexturePtr = m_RenderTargetTexture->GetTextureResourceComPtr();

	m_DepthBufferTexture = depthBufferTexture;
	if (m_DepthBufferTexture == nullptr)
	{
		m_DepthBufferTexture = std::make_shared<Texture>();
	}

	m_SampleCount = sampleCount; //The primary back buffer will never have MSAA. There will be a separate render target that resolves to this when presenting if MSAA is "enabled".

	if (!Resize(m_Width, m_Height))
		return false;

	return true;
}

Texture* RenderTarget::GetDepthBufferTexturePtr()
{
	return m_DepthBufferTexture.get();
}

std::shared_ptr<Texture> RenderTarget::GetSharedTexturePtr()
{
	return m_RenderTargetTexture;
}

RenderTarget::~RenderTarget()
{
	m_RenderTargetView.Reset();
	m_DepthStencilView.Reset();
	long cnt = m_DepthBufferTexture.use_count();
	m_TexturePtr = nullptr;
	OutputDebugStringA("~RenderTarget()");
}

bool RenderTarget::RebuildTexture()
{
	if (m_RenderTargetTexture == nullptr) //For primary render target, need to reassign texture ptr to new swapchain backbuffer
	{
		auto pSwapchain = m_WindowPtr->GetSwapChainPtr();
		if (pSwapchain == nullptr)
		{
			return false;
		}

		//Get backbuffer from swapchain so that we can create primary render target view for window
		ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = pSwapchain->GetBuffer(0,
										   IID_PPV_ARGS(&backBuffer));
		ReturnFalseIfFail(hr, "D3D11 Swapchain backbuffer retrieval failed.");

		D3D11_TEXTURE2D_DESC desc;
		backBuffer->GetDesc(&desc);

		m_TextureFormat = desc.Format;
		m_TexturePtr = backBuffer;
#ifdef _DEBUG
		std::string debugObjectName = "RenderTargetTexture_SwapchainBackBuffer_Window" + std::to_string(m_WindowPtr->GetId());
		m_TexturePtr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)debugObjectName.length(), debugObjectName.c_str());
#endif
	}
	else
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		// Initialize the render target texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		// Setup the render target texture description.
		textureDesc.Width = m_Width;
		textureDesc.Height = m_Height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = m_TextureFormat;
		textureDesc.SampleDesc.Count = m_SampleCount;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		bool multiSampled = (m_SampleCount == 1) ? false : true;

		if (!m_RenderTargetTexture->Initialize(textureDesc, multiSampled))
		{
			ErrorHandler::LogCriticalError("Failed to initialize render target texture for render target.");
			return false;
		}
		m_TexturePtr = m_RenderTargetTexture->GetTextureResourceComPtr();
#ifdef _DEBUG
		std::string debugObjectName = "RenderTargetTexture_";
		m_TexturePtr->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)debugObjectName.length(), debugObjectName.c_str());
#endif
	}
	return true;
}

bool RenderTarget::RebuildRenderTargetView()
{
	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	if (pDevice == nullptr)
	{
		ErrorHandler::LogCriticalError("There was an issue retrieving the device pointer when rebuilding the render target view.");
		return false;
	}

	HRESULT hr;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_TextureFormat;
	if (m_SampleCount == 1)
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	else
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	rtvDesc.Texture2D.MipSlice = 0;

	hr = pDevice->CreateRenderTargetView(m_TexturePtr.Get(),
										 &rtvDesc,
										 &m_RenderTargetView);
	ReturnFalseIfFail(hr, "D3D11 Failed to create render target view for render target.");
	return true;
}

bool RenderTarget::RebuildDepthStencil()
{
	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	if (pDevice == nullptr)
	{
		ErrorHandler::LogCriticalError("There was an issue retrieving the device pointer when rebuilding the render target view.");
		return false;
	}

	//Describe our Depth/Stencil Buffer
	CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
												  m_Width, m_Height); //24 bits for depth, 8 bits for stencil
	depthStencilTextureDesc.MipLevels = 1;
	depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilTextureDesc.SampleDesc.Count = m_SampleCount;
	depthStencilTextureDesc.SampleDesc.Quality = 0;

	bool multiSampled = (m_SampleCount == 1) ? false : true;

	if (!m_DepthBufferTexture->Initialize(depthStencilTextureDesc, multiSampled))
	{
		ErrorHandler::LogCriticalError("Failed to initialize depth buffer texture for render target.");
		return false;
	}

#ifdef _DEBUG
	std::string debugObjectName = "RenderTargetDepthTexture_";
	m_DepthBufferTexture->GetTextureResourceComPtr()->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)debugObjectName.length(), debugObjectName.c_str());
#endif

	HRESULT hr = S_OK;

	hr = pDevice->CreateDepthStencilView(m_DepthBufferTexture->GetTextureResource(),
										 NULL,
										 &m_DepthStencilView);
	ReturnFalseIfFail(hr, "D3D11 Failed to create depth stencil view for render target.");

	return true;
}

bool RenderTarget::Resize(UINT width, UINT height)
{
	m_Width = width;
	m_Height = height;
	if (m_RenderTargetView != nullptr)
		m_RenderTargetView.Reset();
	m_DepthStencilView.Reset();

	if (!RebuildTexture())
	{
		ErrorHandler::LogCriticalError("Failed to rebuild the texture when resizing render target.");
		return false;
	}

	if (!RebuildRenderTargetView())
	{
		ErrorHandler::LogCriticalError("Failed to rebuild the render target view when resizing render target.");
		return false;
	}

	if (!RebuildDepthStencil())
	{
		ErrorHandler::LogCriticalError("Failed to rebuild the depth stencil buffer/view when resizing render target.");
		return false;
	}

	return true;
}
