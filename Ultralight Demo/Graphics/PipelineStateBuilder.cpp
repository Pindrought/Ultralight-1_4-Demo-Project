#include "PCH.h"
#include "PipelineStateBuilder.h"
#include "Renderer.h"
#include "InputLayoutDescriptions.h"

bool PipelineStateBuilder::BuildPipelineStatesForRenderer()
{
	if (!BuildPipelineState2D())
	{
		return false;
	}
	if (!BuildPipelineState3D())
	{
		return false;
	}
    return true;
}

bool PipelineStateBuilder::BuildPipelineState2D()
{
	Renderer* pRenderer = Renderer::GetInstance();

	std::shared_ptr<PipelineState> pipelineState_2d = std::make_shared<PipelineState>("2D");

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	HRESULT hr = S_OK;
	//Depth Stencil State
	CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(D3D11_DEFAULT);
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	ComPtr<ID3D11DepthStencilState> depthStencilState;
	hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	ReturnFalseIfFail(hr, "D3D11 Failed to create depth stencil state.");

	//Rasterizer State
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.MultisampleEnable = TRUE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	ComPtr<ID3D11RasterizerState> rasterizerState;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	ReturnFalseIfFail(hr, "D3D11 Failed to create rasterizer state.");

	//Sampler state
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ComPtr<ID3D11SamplerState> samplerState;
	hr = pDevice->CreateSamplerState(&sampDesc, &samplerState); //Create sampler state
	ReturnFalseIfFail(hr, "D3D11 Failed to initialize sampler state.");

	//Blend state
	D3D11_BLEND_DESC desc = {};
	D3D11_RENDER_TARGET_BLEND_DESC rtdesc = {};
	rtdesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	rtdesc.BlendEnable = TRUE;
	//this src/dest/blend op results in...
	// a = alpha
	// (sourceColor * a) + (destinationColor * (1-a))

	rtdesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtdesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtdesc.BlendOp = D3D11_BLEND_OP_ADD;
	//Always just using 1 for the alpha since i'm not concerned with blending alpha but just the colors based on alpha
	rtdesc.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	rtdesc.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	rtdesc.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

	desc.RenderTarget[0] = rtdesc;
	ComPtr<ID3D11BlendState> blendState;
	hr = pDevice->CreateBlendState(&desc, &blendState);
	ReturnFalseIfFail(hr, "D3DClass::InitializeBlendStates failed to create enabled blend state");

	pipelineState_2d->DepthStencilState = depthStencilState;
	pipelineState_2d->RasterizerState = rasterizerState;
	pipelineState_2d->SamplerState = samplerState;
	pipelineState_2d->BlendState = blendState;
	pipelineState_2d->VertexShader = std::make_shared<VertexShader>();
	pipelineState_2d->PixelShader = std::make_shared<PixelShader>();

	if (!pipelineState_2d->VertexShader->Initialize(L"VS_2D.cso",
													InputLayoutDescription::Default2D))
		return false;

	if (!pipelineState_2d->PixelShader->Initialize(L"PS_2D.cso"))
		return false;

	pipelineState_2d->StencilRef = 0;

	pRenderer->RegisterPipelineState(pipelineState_2d);

	return true;
}

bool PipelineStateBuilder::BuildPipelineState3D()
{
	Renderer* pRenderer = Renderer::GetInstance();

	std::shared_ptr<PipelineState> pipelineState_3d = std::make_shared<PipelineState>("3D");

	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();

	HRESULT hr = S_OK;
	//Depth Stencil State
	CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(D3D11_DEFAULT);
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	ComPtr<ID3D11DepthStencilState> depthStencilState;
	hr = pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	ReturnFalseIfFail(hr, "D3D11 Failed to create depth stencil state.");

	//Rasterizer State
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.MultisampleEnable = TRUE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	ComPtr<ID3D11RasterizerState> rasterizerState;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	ReturnFalseIfFail(hr, "D3D11 Failed to create rasterizer state.");

	//Sampler state
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ComPtr<ID3D11SamplerState> samplerState;
	hr = pDevice->CreateSamplerState(&sampDesc, &samplerState); //Create sampler state
	ReturnFalseIfFail(hr, "D3D11 Failed to initialize sampler state.");

	//Blend state
	D3D11_BLEND_DESC desc = {};
	D3D11_RENDER_TARGET_BLEND_DESC rtdesc = {};
	rtdesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	rtdesc.BlendEnable = TRUE;
	//this src/dest/blend op results in...
	// a = alpha
	// (sourceColor * a) + (destinationColor * (1-a))

	rtdesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtdesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtdesc.BlendOp = D3D11_BLEND_OP_ADD;
	//Always just using 1 for the alpha since i'm not concerned with blending alpha but just the colors based on alpha
	rtdesc.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	rtdesc.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	rtdesc.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;

	desc.RenderTarget[0] = rtdesc;
	ComPtr<ID3D11BlendState> blendState;
	hr = pDevice->CreateBlendState(&desc, &blendState);
	ReturnFalseIfFail(hr, "D3DClass::InitializeBlendStates failed to create enabled blend state");

	pipelineState_3d->DepthStencilState = depthStencilState;
	pipelineState_3d->RasterizerState = rasterizerState;
	pipelineState_3d->SamplerState = samplerState;
	pipelineState_3d->BlendState = blendState;
	pipelineState_3d->VertexShader = std::make_shared<VertexShader>();
	pipelineState_3d->PixelShader = std::make_shared<PixelShader>();

	if (!pipelineState_3d->VertexShader->Initialize(L"VS_3D.cso",
													InputLayoutDescription::Default3D))
		return false;

	if (!pipelineState_3d->PixelShader->Initialize(L"PS_3D.cso"))
		return false;

	pipelineState_3d->StencilRef = 0;

	pRenderer->RegisterPipelineState(pipelineState_3d);

	//Wireframe
	shared_ptr<PipelineState> pipelineState_3d_wireframe = make_shared<PipelineState>("3D WIREFRAME");
	pipelineState_3d_wireframe->ClonePipelineStateProperties(pipelineState_3d);

	CD3D11_RASTERIZER_DESC rasterizerDesc_wireframe(D3D11_DEFAULT);
	rasterizerDesc_wireframe.CullMode = D3D11_CULL_BACK;
	rasterizerDesc_wireframe.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	rasterizerDesc_wireframe.MultisampleEnable = TRUE;
	rasterizerDesc_wireframe.AntialiasedLineEnable = TRUE;
	ComPtr<ID3D11RasterizerState> rasterizerState_wireframe;
	hr = pDevice->CreateRasterizerState(&rasterizerDesc_wireframe, &rasterizerState_wireframe);
	ReturnFalseIfFail(hr, "D3D11 Failed to create wireframe rasterizer state.");

	pipelineState_3d_wireframe->RasterizerState = rasterizerState_wireframe;

	pRenderer->RegisterPipelineState(pipelineState_3d_wireframe);

	return true;
}
