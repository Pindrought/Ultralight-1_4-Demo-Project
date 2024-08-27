#pragma once
#include "PCH.h"
#include "Mesh.h"
#include "../EZGLTF/Skeleton.h"
#include "../EZGLTF/AABB.h"
#include "../EZGLTF/AnimationClip.h"

class Model 
{
public:
	bool Initialize();
	bool Initialize(string inFilePath, string& outErrorMsg);
	EZGLTF::Skeleton& GetSkeleton();
	void AddMesh(std::shared_ptr<Mesh> mesh, int nodeIndex = 0);
	EZGLTF::AnimationClip* GetAnimationClip(std::string animationName);

	void SetSkeleton(EZGLTF::Skeleton& skel);

	EZGLTF::AABB m_AABB;
	std::vector<EZGLTF::AnimationClip> m_Animations;
	std::vector<std::shared_ptr<Mesh>> m_Meshes;

private:
	string m_Name = "";
	EZGLTF::Skeleton m_Skeleton;
};