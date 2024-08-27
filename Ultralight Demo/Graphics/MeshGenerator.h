#pragma once
#include "PCH.h"
#include "Renderable/Mesh.h"

class MeshGenerator
{
public:
	enum MeshGenerationOption
	{
		DEFAULT,
		COLOREDVERTICES,
		TEXTUREDVERTICES
	};
	static shared_ptr<Mesh> GenerateCube(MeshGenerationOption options = MeshGenerationOption::DEFAULT);
};