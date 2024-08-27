#pragma once
#include "PCH.h"
#include "FloatKeys.h"

namespace EZGLTF
{
	enum class AnimationInterpolationType : int32_t
	{
		UNDEFINED,
		LINEAR,
		STEP,
		CUBLICSPLINE
	};

	class AnimationChannel
	{
	public:
		int m_TargetNode = -1;
		AnimationInterpolationType m_RotationInterpolationType = AnimationInterpolationType::UNDEFINED;
		vector<Float4Key> m_RotationKeys;
		AnimationInterpolationType m_TranslationInterpolationType = AnimationInterpolationType::UNDEFINED;
		vector<Float3Key> m_TranslationKeys;
		AnimationInterpolationType m_ScaleInterpolationType = AnimationInterpolationType::UNDEFINED;
		vector<Float3Key> m_ScaleKeys;
		AnimationInterpolationType m_WeightInterpolationType = AnimationInterpolationType::UNDEFINED;
		vector<FloatKey> m_WeightKeys;
	public:
		DirectX::SimpleMath::Matrix SampleNodeTransform(float timeStamp);
		float SampleWeightInterpolation(float timeStamp);
	private:
		DirectX::XMVECTOR SampleRotation(float timeStamp);
		DirectX::XMVECTOR InterpolateFloat4KeysNormalized(Float4Key& prevKey,
														  Float4Key& nextKey,
														  float timeStamp);
		DirectX::XMVECTOR SampleTranslation(float timeStamp);
		DirectX::XMVECTOR SampleScale(float timeStamp);
		DirectX::XMVECTOR InterpolateFloat3Keys(Float3Key& prevKey,
												Float3Key& nextKey,
												float timeStamp);
		static Float4Key DefaultInitialRotationQuaternionKey;
		static Float3Key DefaultInitialTranslationKey;
		static Float3Key DefaultInitialScaleKey;
		static FloatKey  DefaultInitialWeightKey;
	};
}