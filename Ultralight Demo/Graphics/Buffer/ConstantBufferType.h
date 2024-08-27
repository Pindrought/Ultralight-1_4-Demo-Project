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

	struct CB_PerFrameData_3D
	{
		Matrix View;
		Matrix Projection;
		Matrix ViewProjection;
	};

	struct CB_PerDrawData_3D
	{
		Matrix ModelMatrix;
	};

	struct CB_Material
	{
		CB_Material()
		{
		}
		Vector4 BaseColor = { 1, 1, 1, 1 };
		//
		float MetallicFactor = 0.0f;
		float RoughnessFactor = 0.0f;
		BOOL HasBaseColorTexture = FALSE;
		BOOL HasColoredVertices = FALSE;
		//
		BOOL HasBones = FALSE;
		BOOL HasNormals = FALSE;
		uint32_t Padding1 = 0;
		uint32_t Padding2 = 0;
	};

}

