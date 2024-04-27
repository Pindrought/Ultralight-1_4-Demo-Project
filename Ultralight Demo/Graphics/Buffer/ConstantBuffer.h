#pragma once
#include <PCH.h>
#include "ConstantBufferType.h"

template<class T>
class ConstantBuffer
{
public:
	ConstantBuffer() {}
	T m_Data;
	ID3D11Buffer* Get()const;
	ID3D11Buffer* const* GetAddressOf()const;
	bool Initialize();
	bool ApplyChanges(int byteCount = -1);
private:
	ComPtr<ID3D11Buffer> m_Buffer;
};