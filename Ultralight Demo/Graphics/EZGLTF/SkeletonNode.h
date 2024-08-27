#pragma once
#include "PCH.h"

namespace EZGLTF
{
	struct SkeletonNode
	{
		std::string Name = "";
		DirectX::XMFLOAT3 Translation = { 0,0,0 };
		DirectX::XMFLOAT4 QuaternionRotation = { 0,0,0,1 };
		DirectX::XMFLOAT3 Scale = { 1,1,1 };
		DirectX::SimpleMath::Matrix  InverseBindTransform = DirectX::XMMatrixIdentity();
		DirectX::SimpleMath::Matrix  FinalBoneTransform = DirectX::XMMatrixIdentity();
		DirectX::SimpleMath::Matrix  LocalTransform = DirectX::XMMatrixIdentity();
		DirectX::SimpleMath::Matrix  DefaultLocalTransform = DirectX::XMMatrixIdentity();
		DirectX::SimpleMath::Matrix  GlobalTransform = DirectX::XMMatrixIdentity();
		int32_t ParentIndex = -1; //-1 = root node
		int32_t BoneId = -1; //-1 = no bone
		int32_t SkinId = -1; //-1 = no skin
		vector<uint32_t> MeshIndices; //Indices of meshes associated with node
		std::vector<int32_t> ChildrenIndices;
	};
}