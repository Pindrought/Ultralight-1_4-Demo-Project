#pragma pack_matrix( row_major )

struct VertexShaderInput
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float4 Color : COLOR;
	float3 Normal : NORMAL;
	uint4 JointIndices : JOINTINDICES;
	float4 JointWeights : JOINTWEIGHTS;
	uint InstanceID : SV_InstanceID;
};

// Ouput from the vertex shader.
struct VertexShaderOutput
{
	float4 PositionVS : VIEWSPACEPOS; // View space position.
	float3 NormalVS : VIEWSPACENORMAL; // View space normal.
	float2 TexCoord : TEXCOORD; // Texture Coordinate.
	float4 Color : COLOR;
	uint InstanceID : SV_InstanceID;
	float4 Position : SV_POSITION; // Clip space position.
};

struct PerFrameData
{
	float4x4 View;
	float4x4 Projection;
	float4x4 ViewProjection;
};

struct PerDrawData
{
	float4x4 Model;
};

struct Material
{
	float4 BaseColor;
	float MetallicFactor;
	float RoughnessFactor;
	bool HasBaseColorTexture;
	bool HasColoredVertices;
	bool HasBones;
	bool HasNormals;
	uint Padding1;
	uint Padding2;
};

cbuffer _PerFrameData : register(b0)
{
	PerFrameData PerFrameDataCB;
};

cbuffer _PerDrawCB : register(b1)
{
	PerDrawData PerDrawDataCB;
};

cbuffer _MaterialCB : register(b2)
{
	Material MaterialCB;
}

Texture2D BaseColorTexture : TEXTURE : register(t0);
SamplerState LinearClampSampler : SAMPLER : register(s0);