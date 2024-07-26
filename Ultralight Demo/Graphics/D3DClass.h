#pragma once
#include <PCH.h>

class D3DClass //Singleton create once then reference in future
{
public:
	static D3DClass* GetInstance();
	static shared_ptr<D3DClass> GetInstanceShared();

	bool Initialize();
	~D3DClass();
	bool IsTearingSupported();
	bool IsSampleCountSupported(int sampleCount);
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	ComPtr<IDCompositionDevice> m_CompositionDevice;
private:
	D3DClass();
	static shared_ptr<D3DClass> s_Instance;
	bool InitializeDeviceAndContext();
	std::set<UINT> m_SampleCountOptions;
	bool m_IsTearingSupported = false;
	static bool s_Initialized;
};