#include <PCH.h>
#include "PipelineState.h"

PipelineState::PipelineState(std::string name)
	:Name(name)
{

}

void PipelineState::ClonePipelineStateProperties(shared_ptr<PipelineState> pipelineStateToCopy)
{
	BlendState = pipelineStateToCopy->BlendState;
	DepthStencilState = pipelineStateToCopy->DepthStencilState;
	VertexShader = pipelineStateToCopy->VertexShader;
	RasterizerState = pipelineStateToCopy->RasterizerState;
	SamplerState = pipelineStateToCopy->SamplerState;
	StencilRef = pipelineStateToCopy->StencilRef;
	VertexShader = pipelineStateToCopy->VertexShader;
	PixelShader = pipelineStateToCopy->PixelShader;
}
