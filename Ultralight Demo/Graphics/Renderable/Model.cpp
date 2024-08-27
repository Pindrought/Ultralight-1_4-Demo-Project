#include "PCH.h"
#include "Model.h"
#include "../EZGLTF/GLTFLoader.h"

using namespace EZGLTF;

bool Model::Initialize()
{
    SkeletonNode node;
    node.Name = "DefaultNode";
    m_Skeleton.m_Nodes.push_back(node);

    return true;
}

bool Model::Initialize(string inFilePath, string& outErrorMsg)
{
    GLTFLoader loader;
    EZGLTF::ParsedGLTFModel parsedModel;
    bool success = loader.LoadGLTFFile(inFilePath, parsedModel);
    if (success == false)
    {
        outErrorMsg = loader.GetErrorMessage();
        return false;
    }

    m_Name = parsedModel.Name;

    m_AABB = parsedModel.AABB;

    m_Animations = parsedModel.Animations;
    m_Skeleton.m_Nodes = parsedModel.Nodes;
    m_Skeleton.m_SkeletonGlobalNode = parsedModel.SkeletonGlobalNode;
    //m_Skeleton.GlobalInverseTransform is built when the skeleton is Initialized()
    m_Skeleton.Initialize();

    for (int i = 0; i < parsedModel.Meshes.size(); i++)
    {
        auto& parsedMesh = parsedModel.Meshes[i];
        shared_ptr<Mesh> mesh = make_shared<Mesh>();
        for (auto& parsedPrim : parsedMesh.Primitives)
        {
            shared_ptr<Primitive> prim = make_shared<Primitive>();
            if (!prim->m_IndexBuffer.Initialize(parsedPrim.Indices))
            {
                outErrorMsg = "Index buffer initialization failed.";
                return false;
            }
            auto& attr = prim->m_PrimaryPrimitiveAttributeSet;
            if (!attr->m_Positions.Initialize(parsedPrim.Positions)) //Should always have position data
            {
                outErrorMsg = "Position vertex buffer initialization failed.";
                return false;
            }
            if (parsedPrim.Normals.size() > 0)
            {
                if (!attr->m_Normals.Initialize(parsedPrim.Normals))
                {
                    outErrorMsg = "Normals vertex buffer initialization failed.";
                    return false;
                }
            }
            if (parsedPrim.TextureCoordinates.size() > 0)
            {
                if (!attr->m_TexCoords.Initialize(parsedPrim.TextureCoordinates))
                {
                    outErrorMsg = "TexCoords vertex buffer initialization failed.";
                    return false;
                }
            }
            if (parsedPrim.Colors.size() > 0)
            {
                if (!attr->m_Colors.Initialize(parsedPrim.Colors))
                {
                    outErrorMsg = "Colors vertex buffer initialization failed.";
                    return false;
                }
            }
            if (parsedPrim.JointIndices.size() > 0)
            {
                if (!attr->m_JointIndices.Initialize(parsedPrim.JointIndices))
                {
                    outErrorMsg = "Joint indices vertex buffer initialization failed.";
                    return false;
                }
                if (!attr->m_JointWeights.Initialize(parsedPrim.JointWeights))
                {
                    outErrorMsg = "Joint weights vertex buffer initialization failed.";
                    return false;
                }
            }
            
            shared_ptr<PrimitiveRef> primRef = make_shared<PrimitiveRef>(prim, parsedPrim.Material);
            mesh->m_PrimitiveRefs.push_back(primRef);
            m_Meshes.push_back(mesh);
        }
    }

    return true;
}

Skeleton& Model::GetSkeleton()
{
    return m_Skeleton;
}

void Model::AddMesh(std::shared_ptr<Mesh> mesh, int nodeIndex)
{
    assert(m_Skeleton.m_Nodes.size() > nodeIndex);
    int meshIndex = m_Meshes.size();
    m_Meshes.push_back(mesh);
    m_Skeleton.m_Nodes[nodeIndex].MeshIndices.push_back(meshIndex);
}

AnimationClip* Model::GetAnimationClip(std::string animationName)
{
    for (int i = 0; i < m_Animations.size(); i++)
    {
        if (m_Animations[i].Name == animationName)
        {
            return &m_Animations[i];
        }
    }
    return nullptr;
}

void Model::SetSkeleton(Skeleton& skel)
{
    m_Skeleton = skel;
}
