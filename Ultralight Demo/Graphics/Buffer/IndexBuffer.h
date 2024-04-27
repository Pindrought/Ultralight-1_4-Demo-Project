#pragma once
#include <PCH.h>

class IndexBuffer
{
public:
	IndexBuffer(const IndexBuffer& rhs);
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
	UINT m_IndexCount = 0;
public:
	std::vector<UINT> m_Data;
	IndexBuffer() {}
	ID3D11Buffer* Get()const;
	ID3D11Buffer* const* GetAddressOf()const;
	UINT IndexCount() const;
	bool Initialize(uint32_t* data, UINT indexCount);
	bool Initialize(vector<uint32_t>& data);
};