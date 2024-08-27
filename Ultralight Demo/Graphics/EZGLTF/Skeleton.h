#pragma once
#include "PCH.h"
#include "SkeletonNode.h"
#include "AnimationClip.h"

namespace EZGLTF
{
	class Skeleton
	{
	public:
		void Initialize();
		void BuildDefaultTransforms();
		void BuildTransformsForAnimation(AnimationClip* pAnimation, float animationTime);
		std::vector<SkeletonNode> m_Nodes;
		std::vector<Matrix> m_BoneTransforms;
		int32_t m_SkeletonGlobalNode = -1;
		Matrix m_GlobalInverseTransform = DirectX::XMMatrixIdentity();
	};
}