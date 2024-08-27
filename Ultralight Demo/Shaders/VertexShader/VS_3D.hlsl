#include "../Common/Common3D.hlsli"

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
	//const float4x4 model = PerDrawDataCB.Model;
	//const float4x4 model = PerObjectDataCB.Model;
	float4x4 mvp = mul(PerDrawDataCB.Model, mul(PerFrameDataCB.View, PerFrameDataCB.Projection));
	
	if (MaterialCB.HasBones == true)
	{
		//const int boneOffset = PerObjectDataCB.BoneTransformLocation + input.InstanceID * PerObjectDataCB.BoneTransformCount;
		//float4x4 skinMatrix = InstancedMatrixTransforms[input.JointIndices[0] + boneOffset] * input.JointWeights[0] +
		//					  InstancedMatrixTransforms[input.JointIndices[1] + boneOffset] * input.JointWeights[1] +
		//					  InstancedMatrixTransforms[input.JointIndices[2] + boneOffset] * input.JointWeights[2] +
		//					  InstancedMatrixTransforms[input.JointIndices[3] + boneOffset] * input.JointWeights[3];
		//float4x4 finalMatrixWVP = mul(mvp, skinMatrix);
		//output.Position = mul(finalMatrixWVP, float4(input.Position, 1.0f));
		//float4x4 finalMatrixWorld = mul(model, skinMatrix);
		//output.NormalVS = normalize(mul(finalMatrixWorld, float4(input.Normal, 0.0f)));
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