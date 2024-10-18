#pragma once
#include "PCH.h"
#include "AABB.h"
#include "AnimationClip.h"
#include "../Graphics/Renderable/Mesh.h"
#include "../Graphics/Texture.h"
#include "SkeletonNode.h"
#include "tiny_gltf/tiny_gltf.h"

namespace EZGLTF
{
	struct ParsedGLTFModel
	{
		struct PrimitiveData
		{
			std::vector<DirectX::SimpleMath::Vector3> Positions;
			std::vector<DirectX::SimpleMath::Vector3> Normals;
			std::vector<DirectX::SimpleMath::Vector2> TextureCoordinates;
			std::vector<DirectX::SimpleMath::Vector4> Colors;
			std::vector<uint32_t> Indices;
			std::vector<JointIndexVec4> JointIndices;
			std::vector<JointWeightsVec4> JointWeights;
			shared_ptr<Material> Material;
		};
		
		struct MeshData
		{
			string Name = "";
			vector<PrimitiveData> Primitives;
		};

		void Reset()
		{
			Name = "";
			Meshes.clear();
			Materials.clear();
			Nodes.clear();
			Animations.clear();
			PositionsInGlobalSpace.clear();
			IndicesForGlobalSpacePositions.clear();
			AABB.Prepare();
			SkeletonGlobalNode = -1;
			BoneCount = 0;
		}
		std::string Name;
		std::vector<MeshData> Meshes;
		std::vector<shared_ptr<Material>> Materials;
		std::vector<SkeletonNode> Nodes;
		std::vector<AnimationClip> Animations;
		std::vector<DirectX::XMFLOAT3> PositionsInGlobalSpace; //Used for calculating AABB and potentially used for static/dynamic mesh gen afterwards
		std::vector<DirectX::XMINT3> IndicesForGlobalSpacePositions; //Used for calculating AABB and potentially used for static/dynamic mesh gen afterwards
		AABB AABB;
		int32_t SkeletonGlobalNode = -1;
		int32_t BoneCount = 0;
	};

	class GLTFLoader
	{
	public:
		bool LoadGLTFFile(std::string inFilePath, ParsedGLTFModel& outModel);
		string GetErrorMessage();
		
	private:
		ParsedGLTFModel* m_ParseModel = nullptr;
		tinygltf::Model m_GLTFModel;
		std::map<int, int> m_LookupOldNodeToNewNodeIndex; //For reasons, the nodes will be remapped and this will store the final remapping.
		std::map<int, int> m_LookupNewNodeToOldNodeIndex; //For convenience storing this, not technically necessary but annoying to iterate through m_LookupOldNodeToNewNodeIndex

		string m_FilePath = "";
		string m_BaseDirectory = "";
		string m_ErrorMessage = "";
		bool ProcessModel();
		bool ProcessNodes();
		bool ProcessMaterials();
		bool ProcessMeshes();
		bool ProcessAnimations();
		void BuildGlobalSpacePositionsAndIndices();
		void SetErrorMessage(string msg);

	};
}