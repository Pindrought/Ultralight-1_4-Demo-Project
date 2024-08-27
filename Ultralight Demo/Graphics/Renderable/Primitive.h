#include "PCH.h"
#include "../Buffer/IndexBuffer.h"
#include "PrimitiveAttributeSet.h"

class Primitive
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

	std::string m_Name = "";
	IndexBuffer m_IndexBuffer;

	std::shared_ptr<PrimitiveAttributeSet> m_PrimaryPrimitiveAttributeSet = std::make_shared<PrimitiveAttributeSet>();
	//std::vector<PrimitiveAttributeSet> m_MorphTargetPrimitiveAttributeSets; //I don't have support in this project for morph targets, but if I add it, it'll be here

	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};