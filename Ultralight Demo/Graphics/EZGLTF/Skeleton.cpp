#include "PCH.h"
#include "Skeleton.h"

using namespace EZGLTF;

void Skeleton::Initialize()
{
	int boneCount = 0;
	for (auto& node : m_Nodes)
	{
		if (node.BoneId != -1)
		{
			boneCount += 1;
		}
	}
	m_BoneTransforms.resize(boneCount);
	BuildDefaultTransforms();
}

void Skeleton::BuildDefaultTransforms()
{
	for (int i = 0; i < m_Nodes.size(); i++)
	{
		auto& node = m_Nodes[i];
		node.LocalTransform = node.DefaultLocalTransform;
	}

	for (int i = 0; i < m_Nodes.size(); i++)
	{
		auto& node = m_Nodes[i];
		if (node.ParentIndex == -1) //root
		{
			node.GlobalTransform = node.LocalTransform;
		}
		else
		{
			auto& parentNode = m_Nodes[node.ParentIndex];
			node.GlobalTransform = node.LocalTransform * parentNode.GlobalTransform;
		}
	}

	if (m_SkeletonGlobalNode != -1)
	{
		m_GlobalInverseTransform = m_Nodes[m_SkeletonGlobalNode].GlobalTransform.Invert();
		for (int i = 0; i < m_Nodes.size(); i++)
		{
			auto& node = m_Nodes[i];
			if (node.BoneId != -1)
			{
				node.FinalBoneTransform = node.InverseBindTransform * node.GlobalTransform * m_GlobalInverseTransform;
				m_BoneTransforms[node.BoneId] = node.FinalBoneTransform;
			}
		}
	}
}

void Skeleton::BuildTransformsForAnimation(AnimationClip* pAnimation, float animationTime)
{
	if (pAnimation == nullptr)
	{
		BuildDefaultTransforms();
		return;
	}

	auto& channels = pAnimation->Channels;

	//Rebuild the local transforms for every node
	for (int i = 0; i < m_Nodes.size(); i++)
	{
		auto& node = m_Nodes[i];
		node.LocalTransform = node.DefaultLocalTransform;
		if (node.ParentIndex == -1) //If this is a root node, the global transform will be whatever its default local transform is
		{
			node.GlobalTransform = node.DefaultLocalTransform;
		}
	}

	//Now go through and build the transforms from animation for every node that is effected
	for (int i = 0; i < pAnimation->Channels.size(); i++)
	{
		auto& channel = pAnimation->Channels[i];
		auto& node = m_Nodes[channel.m_TargetNode];

		//Need to do this in initialization later, but its fine for now.
		//I need to make sure i'm not overwriting the local transform data if a channel exists for a path
		if (channel.m_RotationKeys.size() == 0)
		{
			channel.m_RotationKeys.push_back(Float4Key(node.QuaternionRotation, 0));
		}
		if (channel.m_TranslationKeys.size() == 0)
		{
			channel.m_TranslationKeys.push_back(Float3Key(node.Translation, 0));
		}
		if (channel.m_ScaleKeys.size() == 0)
		{
			channel.m_ScaleKeys.push_back(Float3Key(node.Scale, 0));
		}

		node.LocalTransform = channel.SampleNodeTransform(animationTime);
		if (node.ParentIndex == -1) //If this is a root node, update the global transform
		{
			node.GlobalTransform = node.LocalTransform;
		}
	}

	//Now go through and rebuild the skeleton using the updated "local" transforms to build the global transforms
	for (int i = 0; i < m_Nodes.size(); i++)
	{
		auto& node = m_Nodes[i];
		if (node.ParentIndex != -1) //Root nodes are already calculated, I don't need to recalculate them
		{
			auto& parentNode = m_Nodes[node.ParentIndex];
			node.GlobalTransform = node.LocalTransform * parentNode.GlobalTransform;
		}
	}

	//Now go through and update all of the final bone matrices
	if (m_SkeletonGlobalNode != -1)
	{
		//m_GlobalInverseTransform = m_Nodes[m_SkeletonGlobalNode].GlobalTransform.Invert();
		for (int i = 0; i < m_Nodes.size(); i++)
		{
			auto& node = m_Nodes[i];
			if (node.BoneId != -1)
			{
				node.FinalBoneTransform = node.InverseBindTransform * node.GlobalTransform * m_GlobalInverseTransform;
				m_BoneTransforms[node.BoneId] = node.FinalBoneTransform;
			}
		}
	}
}