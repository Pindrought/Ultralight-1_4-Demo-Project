#pragma once
#include "Primitive.h"
#include "PCH.h"
#include "../EZGLTF/EZGLTFTypes.h"

class PrimitiveRef
{
public:
	PrimitiveRef();
	PrimitiveRef(std::shared_ptr<Primitive> prim, std::shared_ptr<EZGLTF::Material> material = nullptr);
	std::shared_ptr<Primitive> m_Primitive = nullptr;
	std::shared_ptr<EZGLTF::Material> m_Material = nullptr;
};