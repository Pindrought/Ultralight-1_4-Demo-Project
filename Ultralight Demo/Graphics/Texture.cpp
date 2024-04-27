#include <PCH.h>
#include "Texture.h"
#include "../Engine.h"

bool Texture::Initialize(D3D11_TEXTURE2D_DESC desc,
						 bool multiSampled)
{
	m_MultiSampled = multiSampled;
	m_Texture.Reset();

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	ComPtr<ID3D11Texture2D> p2DTexture = nullptr;
	auto hr = pDevice->CreateTexture2D(&desc, nullptr, &p2DTexture);
	ReturnFalseIfFail(hr, "Failed to create 2d texture from texture description.");

	m_Texture = p2DTexture;

	if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
	{
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, desc.Format);
		if (m_MultiSampled)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		switch (desc.Format)
		{
		case DXGI_FORMAT::DXGI_FORMAT_D16_UNORM:
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS;
			break;
		case DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT:
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
			break;
		case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT:
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
			break;
		case DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS;
			break;
		}
		hr = pDevice->CreateShaderResourceView(m_Texture.Get(),
											   &srvDesc,
											   &m_TextureView);
		ReturnFalseIfFail(hr, "Failed to create 2d texture shader resource view from texture description.");
	}
	return true;
}

Texture::~Texture()
{
	m_Texture.Reset();
	m_TextureView.Reset();
}

ComPtr<ID3D11Resource> Texture::GetTextureResourceComPtr()
{
	return m_Texture;
}

ID3D11Resource* Texture::GetTextureResource()
{
	return m_Texture.Get();
}

ID3D11ShaderResourceView* Texture::GetTextureResourceView()
{
	return m_TextureView.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceViewAddress()
{
	return m_TextureView.GetAddressOf();
}