#pragma once
#include <PCH.h>

class VertexShader
{
public:
	bool Initialize(std::wstring shaderpath,
					std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc);
	ID3D11VertexShader* GetShader();
	ID3D10Blob* GetBuffer();
	ID3D11InputLayout* GetInputLayout();
private:
	ComPtr<ID3D11VertexShader> m_Shader;
	ComPtr<ID3D10Blob> m_ShaderBlob;
	ComPtr<ID3D11InputLayout> m_InputLayout;
};