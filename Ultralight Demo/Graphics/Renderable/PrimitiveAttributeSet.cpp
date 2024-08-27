#include "PCH.h"
#include "PrimitiveAttributeSet.h"

std::vector<ID3D11Buffer*> PrimitiveAttributeSet::GetVertexBuffersVector()
{
    std::vector<ID3D11Buffer*> vertexBuffers;
    vertexBuffers.resize((int)PrimitiveAttributeSet::BufferIndex::DEFAULT_VERTEXBUFFER_COUNT, nullptr);
    if (m_Positions.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::POSITION] = m_Positions.Get();
    if (m_Normals.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::NORMAL] = m_Normals.Get();
    if (m_Colors.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::COLOR] = m_Colors.Get();
    if (m_TexCoords.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::TEXTURECOORDINATES] = m_TexCoords.Get();
    if (m_JointIndices.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::JOINTINDICES] = m_JointIndices.Get();
    if (m_JointWeights.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::JOINTWEIGHTS] = m_JointWeights.Get();
    return vertexBuffers;
}

std::vector<ID3D11Buffer*> PrimitiveAttributeSet::GetVertexBuffersVectorDepthPass()
{
    std::vector<ID3D11Buffer*> vertexBuffers;
    vertexBuffers.resize(4, nullptr);
    if (m_Positions.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::POSITION] = m_Positions.Get();
    if (m_TexCoords.VertexCount() != 0)
        vertexBuffers[(int)PrimitiveAttributeSet::BufferIndex::TEXTURECOORDINATES] = m_TexCoords.Get();
    if (m_JointIndices.VertexCount() != 0)
        vertexBuffers[2] = m_JointIndices.Get();
    if (m_JointWeights.VertexCount() != 0)
        vertexBuffers[3] = m_JointWeights.Get();
    return vertexBuffers;
}

std::vector<UINT> PrimitiveAttributeSet::GetStridesVector()
{
    std::vector<UINT> strides;
    strides.resize((int)PrimitiveAttributeSet::BufferIndex::DEFAULT_VERTEXBUFFER_COUNT);
    strides[(int)PrimitiveAttributeSet::BufferIndex::COLOR] = sizeof(DirectX::XMFLOAT4);
    strides[(int)PrimitiveAttributeSet::BufferIndex::POSITION] = sizeof(DirectX::XMFLOAT3);
    strides[(int)PrimitiveAttributeSet::BufferIndex::NORMAL] = sizeof(DirectX::XMFLOAT3);
    strides[(int)PrimitiveAttributeSet::BufferIndex::TEXTURECOORDINATES] = sizeof(DirectX::XMFLOAT2);
    strides[(int)PrimitiveAttributeSet::BufferIndex::JOINTINDICES] = sizeof(EZGLTF::JointIndexVec4);
    strides[(int)PrimitiveAttributeSet::BufferIndex::JOINTWEIGHTS] = sizeof(EZGLTF::JointWeightsVec4);
    return strides;
}

std::vector<UINT> PrimitiveAttributeSet::GetOffsetsVector()
{
    std::vector<UINT> offsets;
    offsets.resize((int)PrimitiveAttributeSet::BufferIndex::DEFAULT_VERTEXBUFFER_COUNT, 0);
    return offsets;
}

std::vector<UINT> PrimitiveAttributeSet::GetOffsetsVectorDepthPass()
{
    std::vector<UINT> offsets;
    offsets.resize(4, 0);
    return offsets;
}

UINT PrimitiveAttributeSet::GetVertexBufferCount()
{
    return (UINT)PrimitiveAttributeSet::BufferIndex::DEFAULT_VERTEXBUFFER_COUNT;
}
