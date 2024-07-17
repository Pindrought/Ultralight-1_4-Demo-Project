#pragma once
#include <PCH.h>

class D3DClass
{
public:
	static D3DClass* GetInstance();
	bool Initialize();
	~D3DClass();
	bool IsTearingSupported();
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	ComPtr<IDCompositionDevice> m_CompositionDevice;
private:
	static D3DClass* s_Instance;
	bool InitializeDeviceAndContext();
	std::set<UINT> m_SampleCountOptions;
	bool m_IsTearingSupported = false;
};