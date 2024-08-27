#include "../Common/Common3D.hlsli"

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	float4x4 mvp = mul(PerDrawDataCB.Model, mul(PerFrameDataCB.View, PerFrameDataCB.Projection));
	
	if (MaterialCB.HasBones == true)
	{
		float4x4 skinMatrix = BoneTransformsCB.BoneTransforms[input.JointIndices[0]] * input.JointWeights[0] +
							  BoneTransformsCB.BoneTransforms[input.JointIndices[1]] * input.JointWeights[1] +
							  BoneTransformsCB.BoneTransforms[input.JointIndices[2]] * input.JointWeights[2] +
							  BoneTransformsCB.BoneTransforms[input.JointIndices[3]] * input.JointWeights[3];
		float4x4 finalMatrixWVP = mul(skinMatrix, mvp);
		output.Position = mul(float4(input.Position, 1.0f), finalMatrixWVP);
		float4x4 finalMatrixWorld = mul(skinMatrix, model);
		output.NormalVS = normalize(mul(float4(input.Normal, 0.0f), finalMatrixWorld));
	}
	else
	{
		output.Position = mul(float4(input.Position, 1.0f), mvp);
		output.NormalVS = normalize(mul(float4(input.Normal, 0.0f), PerDrawDataCB.Model));
	}

	output.TexCoord = input.TexCoord;
	output.Color = input.Color;
	return output;
};