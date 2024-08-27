#pragma once
#include <PCH.h>
#include "../Buffer/VertexBuffer.h"
#include "../EZGLTF/EZGLTFTypes.h"

class PrimitiveAttributeSet
{
public:
	enum class BufferIndex : int
	{
		POSITION,
		TEXTURECOORDINATES,
		COLOR,
		NORMAL,
		JOINTINDICES,
		JOINTWEIGHTS,
		DEFAULT_VERTEXBUFFER_COUNT
	};
	std::vector<ID3D11Buffer*> GetVertexBuffersVector();
	std::vector<ID3D11Buffer*> GetVertexBuffersVectorDepthPass();

	std::vector<UINT> GetStridesVector();

	std::vector<UINT> GetOffsetsVector();
	std::vector<UINT> GetOffsetsVectorDepthPass();

	UINT GetVertexBufferCount();

	VertexBuffer<Vector3> m_Positions;
	VertexBuffer<Vector4> m_Colors;
	VertexBuffer<Vector3> m_Normals;
	VertexBuffer<Vector2> m_TexCoords;
	VertexBuffer<EZGLTF::JointIndexVec4> m_JointIndices;
	VertexBuffer<EZGLTF::JointWeightsVec4> m_JointWeights;
};

