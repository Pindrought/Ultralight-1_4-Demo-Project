#include "../Common/Common2D.hlsli"

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float4 sampleColor = DiffuseTexture.Sample(LinearClampSampler, input.TexCoord);
	return sampleColor;
}