#pragma once
#include "PCH.h"

namespace EZGLTF
{
	struct SkeletonPose
	{
		std::vector<Matrix> NodeTransforms;
		std::vector<Matrix> BoneTransforms;
	};
}