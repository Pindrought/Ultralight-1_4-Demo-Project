#pragma once
#include <PCH.h>

namespace ConstantBufferType
{
	struct CB_PerFrameData_2D
	{
		DirectX::XMFLOAT4X4 OrthoMatrix;
	};

	struct CB_PerDrawData_2D
	{
		DirectX::XMFLOAT4X4 ModelMatrix;
	};

	struct CB_UltralightData
	{
		DirectX::XMFLOAT4 State;
		DirectX::XMMATRIX Transform;
		DirectX::XMFLOAT4 Scalar4[2];
		DirectX::XMFLOAT4 Vector[8];
		uint32_t ClipSize;
		DirectX::XMMATRIX Clip[8];
	};
}

