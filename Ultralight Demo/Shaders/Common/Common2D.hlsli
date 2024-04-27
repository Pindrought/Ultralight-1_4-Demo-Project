struct VertexShaderInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 Position : SV_POSITION; 
	float2 TexCoord : TEXCOORD; 
};

struct PerRenderTargetData
{
	float4x4 OrthoMatrix;
};

struct PerDrawData
{
	float4x4 Model;
};

cbuffer _PerRenderTargetCB : register(b0)
{
	PerRenderTargetData PerRenderTargetCB;
};

cbuffer _PerDrawCB : register(b1)
{
	PerDrawData PerDrawCB;
};

Texture2D DiffuseTexture : TEXTURE : register(t0);
SamplerState LinearClampSampler : SAMPLER : register(s0);