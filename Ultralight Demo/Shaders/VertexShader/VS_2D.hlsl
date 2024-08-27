#include "../Common/Common2D.hlsli"

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	//float4x4 finalMatrix = mul(PerDrawCB.Model, PerRenderTargetCB.OrthoMatrix); //<-Version I would use if I had transposed the matrices on CPU side
	float4x4 finalMatrix = mul(PerDrawCB.Model, PerRenderTargetCB.OrthoMatrix);
	
	//output.Position = mul(float4(input.Position, 1.0f), finalMatrix); //<-Version I would use if I had transposed the matrices on CPU side
	output.Position = mul(float4(input.Position, 1.0f), finalMatrix);
	
	output.TexCoord = input.TexCoord;
	return output;
};