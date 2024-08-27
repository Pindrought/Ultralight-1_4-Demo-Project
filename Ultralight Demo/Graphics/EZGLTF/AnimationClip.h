#pragma once
#include "PCH.h"
#include "AnimationChannel.h"

namespace EZGLTF
{
	class AnimationClip
	{
	public:
		std::string Name = "";
		std::vector<AnimationChannel> Channels;
		float Duration = 0;
	};
}