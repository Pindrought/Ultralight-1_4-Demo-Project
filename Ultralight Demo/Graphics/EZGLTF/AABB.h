#pragma once
#include "PCH.h"

namespace EZGLTF
{
	struct AABB
	{
		DirectX::XMFLOAT3 GetCenter()
		{
			return {
				MinCoord.x + (MaxCoord.x - MinCoord.x) / 2,
				MinCoord.y + (MaxCoord.y - MinCoord.y) / 2,
				MinCoord.z + (MaxCoord.z - MinCoord.z) / 2
			};
		}
		float GetRadius() //This is assuming the center of AABB is 0,0,0 just to get an estimated radius
		{
			DirectX::XMFLOAT3 center = GetCenter();
			float x = MaxCoord.x - center.x;
			float y = MaxCoord.y - center.y;
			float z = MaxCoord.z - center.z;
			float d = sqrt(x * x + y * y + z * z);
			return d;
		}
		void Prepare()
		{
			MinCoord = DirectX::XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
			MaxCoord = DirectX::XMFLOAT3(FLT_MIN, FLT_MIN, FLT_MIN);
		}
		void ProcessCoord(DirectX::XMFLOAT3& coord)
		{
			MinCoord.x = std::min(coord.x, MinCoord.x);
			MinCoord.y = std::min(coord.y, MinCoord.y);
			MinCoord.z = std::min(coord.z, MinCoord.z);
			MaxCoord.x = std::max(coord.x, MaxCoord.x);
			MaxCoord.y = std::max(coord.y, MaxCoord.y);
			MaxCoord.z = std::max(coord.z, MaxCoord.z);
		}
		DirectX::XMFLOAT3 MinCoord;
		DirectX::XMFLOAT3 MaxCoord;
	};
}