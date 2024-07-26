#include "PCH.h"
#include "D3DClass.h"

shared_ptr<D3DClass> D3DClass::s_Instance = nullptr;
bool D3DClass::s_Initialized = false;

D3DClass* D3DClass::GetInstance()
{
	if (s_Instance == nullptr)
	{
		return GetInstanceShared().get();
	}
	return s_Instance.get();
}

shared_ptr<D3DClass> D3DClass::GetInstanceShared()
{
	if (s_Instance == nullptr)
	{
		//Workaround to pass private constructor into make_shared from https://stackoverflow.com/a/25069711
		struct make_shared_enabler : public D3DClass {};

		s_Instance = make_shared<make_shared_enabler>();
	}
	return s_Instance;
}


bool D3DClass::Initialize()
{
	if (s_Initialized == true)
	{
		return true; //Already initialized
	}


	if (!InitializeDeviceAndContext())
		return false;

	s_Initialized = true;
	return true;
}

D3DClass::~D3DClass()
{
}

bool D3DClass::IsTearingSupported()
{
	return m_IsTearingSupported;
}

bool D3DClass::IsSampleCountSupported(int sampleCount)
{
	return m_SampleCountOptions.contains(sampleCount);
}

bool D3DClass::InitializeDeviceAndContext()
{
	HRESULT hr;

	const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	UINT deviceFlag = 0;
#ifdef _DEBUG
	deviceFlag = D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif
	hr = D3D11CreateDevice(NULL,
						   D3D_DRIVER_TYPE_HARDWARE,
						   NULL,
						   deviceFlag,
						   &featureLevel,
						   1,
						   D3D11_SDK_VERSION,
						   &m_Device,
						   NULL,
						   &m_Context);
	ReturnFalseIfFail(hr, "D3D11 Device/DeviceContext creation failed.");
	UINT qualityLevels = 0;

	for (UINT i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i *= 2)
	{
		hr = m_Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
													 i,
													 &qualityLevels);
		if (SUCCEEDED(hr))
		{
			if (qualityLevels > 0)
				m_SampleCountOptions.insert(i);
		}
	}

	ComPtr<IDXGIFactory4> factory4;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory4));
	BOOL allowTearing = FALSE;
	if (SUCCEEDED(hr))
	{
		ComPtr<IDXGIFactory5> factory5;
		hr = factory4.As(&factory5);
		if (SUCCEEDED(hr))
		{
			hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
	}
	m_IsTearingSupported = SUCCEEDED(hr) && allowTearing;

	ComPtr<IDXGIDevice2> dxgiDevice;
	hr = m_Device.As(&dxgiDevice);
	ReturnFalseIfFail(hr, "Failed to retrieve underlying DXGI Device from D3D11 Device.");

	hr = DCompositionCreateDevice(dxgiDevice.Get(),
								  __uuidof(m_CompositionDevice),
								  &m_CompositionDevice);
	ReturnFalseIfFail(hr, "Failed to create direct composition device.");

	return true;
}

D3DClass::D3DClass() {}