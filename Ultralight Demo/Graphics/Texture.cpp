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

bool Texture::Initialize(const string& filePath, bool forceSRGB)
{
	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();
	std::string ext = filePath.substr(filePath.find_last_of(".") + 1);
	if (ext == "dds")
	{
		HRESULT hr = E_FAIL;
		DirectX::ScratchImage scratch;
		hr = DirectX::LoadFromDDSFile(StringConverter::s2ws(filePath).c_str(),
									  DirectX::DDS_FLAGS::DDS_FLAGS_NONE,
									  nullptr,
									  scratch);
		if (FAILED(hr))
			return false;

		const DirectX::Image* image = scratch.GetImage(0, 0, 0);
		assert(image);

		CD3D11_TEXTURE2D_DESC textureDesc(image->format, image->width, image->height);
		textureDesc.MipLevels = 1;
		ID3D11Texture2D* p2DTexture = nullptr;
		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = image->pixels;
		initialData.SysMemPitch = (UINT)image->rowPitch;
		hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
		if (FAILED(hr))
			return false;

		m_Texture = static_cast<ID3D11Texture2D*>(p2DTexture);
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
		hr = pDevice->CreateShaderResourceView(m_Texture.Get(),
											   &srvDesc,
											   &m_TextureView);
		if (FAILED(hr))
			return false;

		return true;

	}
	else if (ext == "tga")
	{
		HRESULT hr = E_FAIL;
		DirectX::ScratchImage scratch;
		hr = DirectX::LoadFromTGAFile(StringConverter::s2ws(filePath).c_str(),
									  nullptr,
									  scratch);
		if (FAILED(hr))
			return false;

		const DirectX::Image* image = scratch.GetImage(0, 0, 0);
		assert(image);

		CD3D11_TEXTURE2D_DESC textureDesc(image->format, image->width, image->height);
		textureDesc.MipLevels = 1;
		ID3D11Texture2D* p2DTexture = nullptr;
		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = image->pixels;
		initialData.SysMemPitch = (UINT)image->rowPitch;
		hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
		if (FAILED(hr))
			return false;

		m_Texture = static_cast<ID3D11Texture2D*>(p2DTexture);
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
		hr = pDevice->CreateShaderResourceView(m_Texture.Get(),
											   &srvDesc,
											   &m_TextureView);
		if (FAILED(hr))
			return false;

		return true;
	}
	else
	{
		HRESULT hr = E_FAIL;

		DirectX::ScratchImage scratch;
		DirectX::TexMetadata metadata;
		hr = DirectX::LoadFromWICFile(StringConverter::s2ws(filePath).c_str(),
									  DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB,
									  &metadata, scratch);
		if (FAILED(hr))
			return false;

		const DirectX::Image* image = scratch.GetImage(0, 0, 0);
		assert(image);

		CD3D11_TEXTURE2D_DESC textureDesc(image->format, image->width, image->height);
		textureDesc.MipLevels = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		ID3D11Texture2D* p2DTexture = nullptr;
		hr = pDevice->CreateTexture2D(&textureDesc, nullptr, &p2DTexture);
		if (FAILED(hr))
			return false;

		m_Texture = static_cast<ID3D11Texture2D*>(p2DTexture);

		ID3D11DeviceContext* pDeviceContext = pD3D->m_Context.Get();
		pDeviceContext->UpdateSubresource(m_Texture.Get(), 0u,
										  nullptr,
										  image->pixels,
										  image->rowPitch, 0);

		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
		hr = pDevice->CreateShaderResourceView(m_Texture.Get(),
											   &srvDesc,
											   &m_TextureView);
		if (FAILED(hr))
			return false;

		pDeviceContext->GenerateMips(m_TextureView.Get());

		if (FAILED(hr))
			return false;

		return true;
	}
	return false;
}

Texture::~Texture()
{
	m_Texture.Reset();
	m_TextureView.Reset();
}

ComPtr<ID3D11Texture2D> Texture::GetTextureResourceComPtr()
{
	return m_Texture;
}

ComPtr<ID3D11ShaderResourceView> Texture::GetTextureResourceViewComPtr()
{
	return m_TextureView;
}

ID3D11Texture2D* Texture::GetTextureResource()
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