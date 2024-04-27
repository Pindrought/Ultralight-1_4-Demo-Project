#pragma once
#include <PCH.h>

class PixelShader
{
public:
	bool Initialize(std::wstring shaderpath);
	ID3D11PixelShader* GetShader();
	ID3D10Blob* GetBuffer();
private:
	ComPtr<ID3D11PixelShader> m_Shader;
	ComPtr<ID3D10Blob> m_ShaderBlob;
};