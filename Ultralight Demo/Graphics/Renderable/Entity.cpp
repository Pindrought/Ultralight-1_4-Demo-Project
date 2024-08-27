#include "PCH.h"
#include "Entity.h"

using namespace DirectX;

void Entity::AdvanceAnimation(float deltaTime)
{
    if (m_ActiveAnimation != nullptr)
    {
        m_AnimationTime += (deltaTime / 1000.0f) * m_AnimationSpeed;
        if (m_AnimationTime > m_ActiveAnimation->Duration)
        {
            m_AnimationTime = fmodf(m_AnimationTime, m_ActiveAnimation->Duration);
        }
    }
}

EZGLTF::AnimationClip* Entity::GetActiveAnimationClip()
{
    return m_ActiveAnimation;
}

Matrix Entity::GetMatrix()
{
    return m_WorldMatrix;
}

Model* Entity::GetModel()
{
    return m_Model.get();
}

Vector3 Entity::GetPosition()
{
    return m_Position;
}

Quaternion Entity::GetRotation()
{
    return m_Quaternion;
}

Vector3 Entity::GetScale()
{
    return m_Scale;
}

bool Entity::IsVisible() const
{
    return m_IsVisible;
}

void Entity::SetModel(shared_ptr<Model> model)
{
    m_Model = model;
}

void Entity::SetPosition(Vector3 pos)
{
    m_Position = pos;
}

void Entity::SetRotation(Quaternion rot)
{
    m_Quaternion = rot;
    UpdateMatrix();
}

void Entity::SetScale(Vector3 scale)
{
    m_Scale = scale;
    UpdateMatrix();
}

void Entity::SetVisibility(bool isVisible)
{
    m_IsVisible = isVisible;
}

void Entity::UpdatePose()
{
    auto& skeleton = m_Model->GetSkeleton();
    skeleton.BuildTransformsForAnimation(m_ActiveAnimation, m_AnimationTime);
    if (m_SkeletonPose.NodeTransforms.size() == 0)
    {
        m_SkeletonPose.NodeTransforms.resize(skeleton.m_Nodes.size());
        m_SkeletonPose.BoneTransforms.resize(skeleton.m_BoneTransforms.size());
    }
    for (int i = 0; i < skeleton.m_Nodes.size(); i++)
    {
        m_SkeletonPose.NodeTransforms[i] = skeleton.m_Nodes[i].GlobalTransform;
    }
    memcpy(m_SkeletonPose.BoneTransforms.data(),
           skeleton.m_BoneTransforms.data(),
           sizeof(Matrix) * m_SkeletonPose.BoneTransforms.size());
}

EZGLTF::SkeletonPose& Entity::GetPose()
{
    return m_SkeletonPose;
}

void Entity::UpdateMatrix()
{
    m_WorldMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
                    XMMatrixRotationQuaternion(m_Quaternion) *
                    XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}
