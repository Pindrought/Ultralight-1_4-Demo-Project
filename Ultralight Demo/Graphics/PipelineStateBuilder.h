#pragma once
#include <PCH.h>

class PipelineStateBuilder
{
public:
	static bool BuildPipelineStatesForRenderer();
private:
	static bool BuildPipelineState2D();
	static bool BuildPipelineState3D();
};