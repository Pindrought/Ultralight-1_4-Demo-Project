#include <PCH.h>
#include "VertexBuffer.h"
#include "Vertex.h"
#include "../D3DClass.h"

template<class T>
ID3D11Buffer* VertexBuffer<T>::Get() const
{
	return m_Buffer.Get();
}

template<class T>
ID3D11Buffer* const* VertexBuffer<T>::GetAddressOf() const
{
	return m_Buffer.GetAddressOf();
}

template<class T>
UINT VertexBuffer<T>::VertexCount() const
{
	return static_cast<UINT>(m_Count);
}

template<class T>
UINT VertexBuffer<T>::Stride() const
{
	return m_Stride;
}


template<class T>
const UINT* VertexBuffer<T>::StridePtr() const
{
	return &m_Stride;
}

template<class T>
bool VertexBuffer<T>::Initialize(std::vector<T> data)
{
	m_Count = data.size();

	if (data.size() == 0) //if empty buffer 
	{
		return E_FAIL;
	}
	else
	{

		D3D11_BUFFER_DESC vertexBufferDesc = {};

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = m_Stride * m_Count;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = data.data();

		ID3D11Device* pDevice = D3DClass::GetInstance()->m_Device.Get();
		HRESULT hr = pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_Buffer);
		ReturnFalseIfFail(hr, "Failed to initialized vertex buffer.");
		return true;
	}
}

template<class T>
bool VertexBuffer<T>::Initialize(T* data,
								 int32_t bufferSize,
								 int32_t stride)
{
	if (stride == -1)
	{
		stride = sizeof(T);
	}
	else
	{
		stride = stride;
	}
	m_Count = bufferSize / stride;

	if (bufferSize == 0) //if empty buffer 
	{
		ErrorHandler::LogCriticalError("Attempted to initialize VertexBuffer with a buffer size of 0.");
		return false;
	}
	else
	{
		D3D11_BUFFER_DESC vertexBufferDesc = {};

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = bufferSize;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = data;

		ID3D11Device* pDevice = D3DClass::GetInstance()->m_Device.Get();
		HRESULT hr = pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_Buffer);
		ReturnFalseIfFail(hr, "Failed to initialized vertex buffer.");
		return true;
	}
}

template VertexBuffer<Vertex_2D>;