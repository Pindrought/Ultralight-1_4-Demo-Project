#pragma once
#include <PCH.h>
#include "../Buffer/VertexBuffer.h"
#include "../Buffer/Vertex.h"
#include "../Buffer/IndexBuffer.h"

class Mesh2D
{
public:
	bool Initialize(vector<Vertex_2D> vertices, vector<uint32_t> indices);
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	const uint32_t GetIndexCount();
	const uint32_t GetStride();
private:
	VertexBuffer<Vertex_2D> m_Vertices;
	IndexBuffer m_Indices;
};