#pragma once
#include <PCH.h>

template<class T>
class VertexBuffer
{
private:
	ComPtr<ID3D11Buffer> m_Buffer;
	UINT m_Stride = sizeof(T);
	UINT m_Count = 0;
public:
	VertexBuffer() {}
	ID3D11Buffer* Get() const;
	ID3D11Buffer* const* GetAddressOf()const;
	UINT VertexCount() const;
	UINT Stride() const;
	const UINT* StridePtr() const;
	bool Initialize(std::vector<T> data);
	bool Initialize(T* data, int32_t bufferSize, int32_t stride = -1);
};