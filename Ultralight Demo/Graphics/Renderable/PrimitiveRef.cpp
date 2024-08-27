#include "PCH.h"
#include "PrimitiveRef.h"

PrimitiveRef::PrimitiveRef()
{
	assert("This constructor should never be called." && 0);
}

PrimitiveRef::PrimitiveRef(std::shared_ptr<Primitive> prim, std::shared_ptr<EZGLTF::Material> material)
{
	m_Primitive = prim;
	m_Material = material;
	if (m_Material == nullptr)
	{
		m_Material = std::make_shared<EZGLTF::Material>();
	}
}