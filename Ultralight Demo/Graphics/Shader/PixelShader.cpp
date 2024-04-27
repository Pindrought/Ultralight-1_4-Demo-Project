#include <PCH.h>
#include "PixelShader.h"
#include "../D3DClass.h"

bool PixelShader::Initialize(std::wstring shaderpath)
{
	std::wstring filePath = DirectoryHelper::GetExecutableDirectory() + shaderpath;

	HRESULT hr = D3DReadFileToBlob(filePath.c_str(), &m_ShaderBlob);
	ReturnFalseIfFail(hr, "Failed to read pixel shader to blob.");

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	hr = pDevice->CreatePixelShader(m_ShaderBlob.Get()->GetBufferPointer(),
									m_ShaderBlob.Get()->GetBufferSize(),
									NULL,
									&m_Shader);
	ReturnFalseIfFail(hr, "Failed to create pixel shader.");

	return true;
}

ID3D11PixelShader* PixelShader::GetShader()
{
	return m_Shader.Get();
}

ID3D10Blob* PixelShader::GetBuffer()
{
	return m_ShaderBlob.Get();
}