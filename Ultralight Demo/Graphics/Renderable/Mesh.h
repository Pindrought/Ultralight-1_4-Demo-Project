#pragma once
#include "PCH.h"
#include "PrimitiveRef.h"
#include "../EZGLTF/AABB.h"

class Mesh
{
public:
	shared_ptr<EZGLTF::AABB> m_AABB = nullptr;
	std::string m_Name = "";
	std::vector<std::shared_ptr<PrimitiveRef>> m_PrimitiveRefs;
};