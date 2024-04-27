#pragma once
#include <PCH.h>

class Texture
{
public:
	bool Initialize(D3D11_TEXTURE2D_DESC desc,
					bool multiSampled = false);
	~Texture();
	ComPtr<ID3D11Resource> GetTextureResourceComPtr();
	ID3D11Resource* GetTextureResource();
	ID3D11ShaderResourceView* GetTextureResourceView();
	ID3D11ShaderResourceView** GetTextureResourceViewAddress();
public:
	ComPtr<ID3D11Resource> m_Texture = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_TextureView = nullptr;
	std::string m_Name = "";
	bool m_MultiSampled = false;
};