#include <PCH.h>
#include "ConstantBuffer.h"
#include "../D3DClass.h"

template<class T>
ID3D11Buffer* ConstantBuffer<T>::Get() const
{
	return m_Buffer.Get();
}

template<class T>
ID3D11Buffer* const* ConstantBuffer<T>::GetAddressOf() const
{
	return m_Buffer.GetAddressOf();
}

template<class T>
bool ConstantBuffer<T>::Initialize()
{
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	if (sizeof(T) % 16 == 0)
	{
		desc.ByteWidth = sizeof(T);
	}
	else
	{
		desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16))); //Align to 16 bytes
	}
	desc.StructureByteStride = 0;

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();
	if (pDevice == nullptr)
	{
		ErrorHandler::LogCriticalError("Constant buffer initialization failed to retrieve device ptr.");
		return false;
	}
	HRESULT hr = pDevice->CreateBuffer(&desc, 0, &m_Buffer);
	ReturnFalseIfFail(hr, "Failed to create buffer for constant buffer.");

	return true;
}

template<class T>
bool ConstantBuffer<T>::ApplyChanges(int byteCount)
{
	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11DeviceContext* pContext = pD3D->m_Context.Get();

	if (pContext == nullptr)
	{
		ErrorHandler::LogCriticalError("Constant buffer failed to retrieve context ptr.");
		return false;
	}
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//mappedResource.pData = buffer;
	auto pBuffer = m_Buffer.Get();
	HRESULT hr = pContext->Map(m_Buffer.Get(),
							   0,
							   D3D11_MAP_WRITE_DISCARD,
							   0,
							   &mappedResource);
	ReturnFalseIfFail(hr, "Failed to map constant buffer.");
	if (byteCount == -1)
	{
		memcpy(mappedResource.pData, &m_Data, sizeof(T));
	}
	else
	{
		memcpy(mappedResource.pData, &m_Data, byteCount);
	}
	pContext->Unmap(m_Buffer.Get(), 0);
	return true;
}

template ConstantBuffer<ConstantBufferType::CB_PerFrameData_2D>;
template ConstantBuffer<ConstantBufferType::CB_PerDrawData_2D>;
template ConstantBuffer<ConstantBufferType::CB_UltralightData>;
template ConstantBuffer<ConstantBufferType::CB_PerFrameData_3D>;
template ConstantBuffer<ConstantBufferType::CB_PerDrawData_3D>;
template ConstantBuffer<ConstantBufferType::CB_Material>;