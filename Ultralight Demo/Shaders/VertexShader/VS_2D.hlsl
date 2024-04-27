#include "../Common/Common2d.hlsli"

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4x4 finalMatrix = mul(PerDrawCB.Model, PerRenderTargetCB.OrthoMatrix);
	//input.Position.x = input.Position.x * 2 - 1;
	//input.Position.y = input.Position.y * 2 - 1;
	
	//output.Position = float4(input.Position, 1.0f);
	output.Position = mul(float4(input.Position, 1.0f), finalMatrix);
	output.TexCoord = input.TexCoord;
	return output;
};