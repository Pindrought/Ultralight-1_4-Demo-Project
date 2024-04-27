#include "PCH.h"
#include "Mesh2D.h"

bool Mesh2D::Initialize(vector<Vertex_2D> vertices, vector<uint32_t> indices)
{
    if (!m_Vertices.Initialize(vertices))
    {
        ErrorHandler::LogCriticalError("Failed to initialize Mesh2D vertex buffer.");
        return false;
    }
    if (!m_Indices.Initialize(indices))
    {
        ErrorHandler::LogCriticalError("Failed to initialize index buffer for Mesh2D.");
        return false;
    }
    return true;
}

ID3D11Buffer* Mesh2D::GetVertexBuffer()
{
    return m_Vertices.Get();
}

ID3D11Buffer* Mesh2D::GetIndexBuffer()
{
    return m_Indices.Get();
}

const uint32_t Mesh2D::GetIndexCount()
{
    return m_Indices.IndexCount();
}

const uint32_t Mesh2D::GetStride()
{
    return m_Vertices.Stride();
}
