#include <PCH.h>
#include "VertexShader.h"
#include "../../../Engine.h"

bool VertexShader::Initialize(std::wstring shaderpath,
							  std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc)
{
	std::wstring filePath = DirectoryHelper::GetExecutableDirectory() + shaderpath;

	HRESULT hr = D3DReadFileToBlob(filePath.c_str(), m_ShaderBlob.GetAddressOf());
	ReturnFalseIfFail(hr, "Failed to read vertex shader to blob.");

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	hr = pDevice->CreateVertexShader(m_ShaderBlob->GetBufferPointer(),
									 m_ShaderBlob->GetBufferSize(),
									 NULL,
									 &m_Shader);
	ReturnFalseIfFail(hr, "Failed to create vertex shader.");

	hr = pDevice->CreateInputLayout(layoutDesc.data(),
									layoutDesc.size(),
									m_ShaderBlob->GetBufferPointer(),
									m_ShaderBlob->GetBufferSize(),
									&m_InputLayout);
	ReturnFalseIfFail(hr, "Failed to create input layout for vertex shader.");

	return true;
}

ID3D11VertexShader* VertexShader::GetShader()
{
	return m_Shader.Get();
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return m_ShaderBlob.Get();
}

ID3D11InputLayout* VertexShader::GetInputLayout()
{
	return m_InputLayout.Get();
}