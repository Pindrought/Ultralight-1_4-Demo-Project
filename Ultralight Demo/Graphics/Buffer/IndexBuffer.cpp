#include <PCH.h>
#include "IndexBuffer.h"
#include "../D3DClass.h"

IndexBuffer::IndexBuffer(const IndexBuffer& rhs)
{
	m_Buffer = rhs.m_Buffer;
	m_IndexCount = rhs.m_IndexCount;
	m_Data = rhs.m_Data;
}

ID3D11Buffer* IndexBuffer::Get() const
{
	return m_Buffer.Get();
}

ID3D11Buffer* const* IndexBuffer::GetAddressOf() const
{
	return m_Buffer.GetAddressOf();
}

UINT IndexBuffer::IndexCount() const
{
	return m_IndexCount;
}

bool IndexBuffer::Initialize(uint32_t* data, UINT indexCount)
{
	if (data != nullptr)
	{
		m_Data.resize(indexCount);
		memcpy(&m_Data[0], data, indexCount * sizeof(UINT));
	}
	m_IndexCount = indexCount;
	//Load Index Data
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = data;
	ID3D11Device* pDevice = D3DClass::GetInstance()->m_Device.Get();
	HRESULT hr = pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_Buffer);
	ReturnFalseIfFail(hr, "Failed to create index buffer.");
	return true;
}

bool IndexBuffer::Initialize(vector<uint32_t>& data)
{
	return Initialize(data.data(), data.size());
}
