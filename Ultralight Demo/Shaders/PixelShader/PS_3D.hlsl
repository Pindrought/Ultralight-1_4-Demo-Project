#include "../Common/Common3D.hlsli"

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float lightStrength = 1;

	if (MaterialCB.HasNormals)
	{
		float3 lightdir = float3(-0.2, -0.35, 0.5);
		lightdir = normalize(lightdir);
		lightdir = -lightdir;

		float ambientStrength = 1.0;
		float NDotL = max(dot(input.NormalVS, lightdir), 0);
		lightStrength = ambientStrength + NDotL * (1 - ambientStrength);
	}

	float4 color = float4(1, 1, 1, 1);

	if (MaterialCB.HasBaseColorTexture)
	{
		color = BaseColorTexture.Sample(LinearClampSampler, input.TexCoord);
	}
	else
	{
		if (MaterialCB.HasColoredVertices)
		{
			color = input.Color * MaterialCB.BaseColor;
		}
		else
		{
			color = MaterialCB.BaseColor;
		}
	}
	
	float3 finalColor = color * lightStrength;
		
	return float4(finalColor, color.a);
}

