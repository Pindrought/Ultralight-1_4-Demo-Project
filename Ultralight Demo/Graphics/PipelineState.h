#pragma once
#include <PCH.h>
#include "Shader/VertexShader.h"
#include "Shader/PixelShader.h"

struct PipelineState
{
public:
	PipelineState() {}
	PipelineState(std::string name);
	std::string Name = "Undefined";
	ComPtr<ID3D11RasterizerState> RasterizerState = nullptr;
	ComPtr<ID3D11SamplerState> SamplerState = nullptr;
	ComPtr<ID3D11DepthStencilState> DepthStencilState = nullptr;
	ComPtr<ID3D11BlendState> BlendState = nullptr;
	UINT StencilRef = 0;
	std::shared_ptr<VertexShader> VertexShader = nullptr;
	std::shared_ptr<PixelShader> PixelShader = nullptr;
private:
};