#pragma once
#include "PCH.h"
#include "../Buffer/ConstantBufferType.h"
#include "../PipelineState.h"

namespace EZGLTF
{

	struct Material
	{
		string Name;
		shared_ptr<Texture> BaseColorTexture = nullptr;
		struct MatProperties
		{
			Vector4 BaseColor = { 1, 1, 1, 1 };
			float MetallicFactor = 0.0f;
			float RoughnessFactor = 0.0f;
		};
		MatProperties Properties;
		shared_ptr<PipelineState> PipelineState = nullptr; //For nullptr pipeline states, just the normal 3D pipeline state is used (See Renderer::RenderEntity())
	};

	struct JointWeightsVec4
	{
		float JointWeight[4];
	};

	struct JointIndexVec4
	{
		uint32_t JointIndex[4];
	};
}