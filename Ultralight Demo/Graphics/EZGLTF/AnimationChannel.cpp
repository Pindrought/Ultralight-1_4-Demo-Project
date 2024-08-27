#include "PCH.h"
#include "AnimationChannel.h"

using namespace EZGLTF;
using namespace DirectX;

Float4Key AnimationChannel::DefaultInitialRotationQuaternionKey = Float4Key(DirectX::XMFLOAT4(0, 0, 0, 1), 0);
Float3Key AnimationChannel::DefaultInitialTranslationKey = Float3Key(DirectX::XMFLOAT3(0, 0, 0), 0);
Float3Key AnimationChannel::DefaultInitialScaleKey = Float3Key(DirectX::XMFLOAT3(1, 1, 1), 0);
FloatKey  AnimationChannel::DefaultInitialWeightKey = FloatKey(0, 0);

DirectX::SimpleMath::Matrix AnimationChannel::SampleNodeTransform(float timeStamp)
{

    auto rotationQuaternion = SampleRotation(timeStamp);
    auto translationVector = SampleTranslation(timeStamp);

    auto scalingVector = SampleScale(timeStamp);
    DirectX::SimpleMath::Matrix nodeTransform = XMMatrixScalingFromVector(scalingVector) *
        XMMatrixRotationQuaternion(rotationQuaternion) *
        XMMatrixTranslationFromVector(translationVector);
    return nodeTransform;

}

float AnimationChannel::SampleWeightInterpolation(float timeStamp)
{
    if (m_WeightKeys.size() == 0)
    {
        return 0;
    }

    for (int i = 0; i < m_WeightKeys.size(); i++)
    {
        if (timeStamp < m_WeightKeys[i].m_AnimationTime)
        {
            if (i == 0) //missing default starting entry where t=0?
            {
                int nextKeyIndex = i;
                FloatKey& nextKey = m_WeightKeys[nextKeyIndex];

                const float range = nextKey.m_AnimationTime;
                const float pct = (timeStamp) / range;

                float interpolatedWeight = nextKey.m_KeyData * (pct);
                return interpolatedWeight;
            }
            else
            {
                int prevKeyIndex = i - 1;
                int nextKeyIndex = i;
                FloatKey& prevKey = m_WeightKeys[prevKeyIndex];
                FloatKey& nextKey = m_WeightKeys[nextKeyIndex];

                const float range = nextKey.m_AnimationTime - prevKey.m_AnimationTime;
                const float pct = (timeStamp - prevKey.m_AnimationTime) / range;

                float interpolatedWeight = nextKey.m_KeyData * (pct)+
                    prevKey.m_KeyData * (1 - pct);
                return interpolatedWeight;
            }
        }
    }

    return 0.0f;
}

XMVECTOR AnimationChannel::SampleRotation(float timeStamp)
{
    if (m_RotationKeys.size() == 1)
    {
        XMVECTOR rotKeyVec = XMLoadFloat4(&m_RotationKeys[0].m_KeyData);
        return rotKeyVec;
    }

    for (int i = 0; i < m_RotationKeys.size(); i++)
    {
        if (timeStamp < m_RotationKeys[i].m_AnimationTime)
        {
            if (i == 0) //some animations don't have a frame for beginning so we'll just use identity quat i guess
            {
                int nextKeyIndex = i;
                Float4Key& prevKey = m_RotationKeys[m_RotationKeys.size() - 1];
                Float4Key& nextKey = m_RotationKeys[nextKeyIndex];
                XMVECTOR interpolatedRotation = InterpolateFloat4KeysNormalized(prevKey, nextKey, timeStamp);
                return interpolatedRotation;
            }
            else
            {
                int prevKeyIndex = i - 1;
                int nextKeyIndex = i;
                Float4Key& prevKey = m_RotationKeys[prevKeyIndex];
                Float4Key& nextKey = m_RotationKeys[nextKeyIndex];
                XMVECTOR interpolatedRotation = InterpolateFloat4KeysNormalized(prevKey, nextKey, timeStamp);
                return interpolatedRotation;
            }
        }
    }

    return DirectX::XMVECTOR();
}

XMVECTOR AnimationChannel::InterpolateFloat4KeysNormalized(Float4Key& prevKey,
                                                            Float4Key& nextKey,
                                                            float timeStamp)
{
    const float range = nextKey.m_AnimationTime - prevKey.m_AnimationTime;
    const float pct = (timeStamp - prevKey.m_AnimationTime) / range;
    XMVECTOR v1 = XMLoadFloat4(&prevKey.m_KeyData);
    XMVECTOR v2 = XMLoadFloat4(&nextKey.m_KeyData);

    auto interpolatedResult = XMQuaternionSlerp(v1, v2, pct);
    auto normalizedResult = DirectX::XMVector4Normalize(interpolatedResult);
    return normalizedResult;
}

XMVECTOR AnimationChannel::SampleTranslation(float timeStamp)
{
    if (m_TranslationKeys.size() == 1)
    {
        XMVECTOR posKeyVec = XMLoadFloat3(&m_TranslationKeys[0].m_KeyData);
        return posKeyVec;
    }

    for (int i = 0; i < m_TranslationKeys.size(); i++)
    {
        if (timeStamp < m_TranslationKeys[i].m_AnimationTime)
        {
            if (i == 0) //If this animation is missing the starting entry
            {
                int nextKeyIndex = i;
                //MW3DFloat3Key& prevKey = DefaultInitialTranslationKey;
                Float3Key& prevKey = m_TranslationKeys[m_TranslationKeys.size() - 1];
                Float3Key& nextKey = m_TranslationKeys[nextKeyIndex];
                XMVECTOR interpolatedPosition = InterpolateFloat3Keys(prevKey, nextKey, timeStamp);
                return interpolatedPosition;
            }
            else
            {
                int prevKeyIndex = i - 1;
                int nextKeyIndex = i;
                Float3Key& prevKey = m_TranslationKeys[prevKeyIndex];
                Float3Key& nextKey = m_TranslationKeys[nextKeyIndex];
                XMVECTOR interpolatedPosition = InterpolateFloat3Keys(prevKey, nextKey, timeStamp);
                return interpolatedPosition;
            }
        }
    }

    return DirectX::XMVECTOR();
}

DirectX::XMVECTOR AnimationChannel::SampleScale(float timeStamp)
{
    if (m_ScaleKeys.size() == 1)
    {
        XMVECTOR posKeyVec = XMLoadFloat3(&m_ScaleKeys[0].m_KeyData);
        return posKeyVec;
    }

    for (int i = 0; i < m_ScaleKeys.size(); i++)
    {
        if (timeStamp < m_ScaleKeys[i].m_AnimationTime)
        {
            if (i == 0) //If this animation is missing the starting entry
            {
                int nextKeyIndex = i;
                // MW3DFloat3Key& prevKey = DefaultInitialScaleKey;
                Float3Key& prevKey = m_ScaleKeys[m_ScaleKeys.size() - 1];
                Float3Key& nextKey = m_ScaleKeys[nextKeyIndex];
                XMVECTOR interpolatedPosition = InterpolateFloat3Keys(prevKey, nextKey, timeStamp);
                return interpolatedPosition;
            }
            else
            {
                int prevKeyIndex = i - 1;
                int nextKeyIndex = i;
                Float3Key& prevKey = m_ScaleKeys[prevKeyIndex];
                Float3Key& nextKey = m_ScaleKeys[nextKeyIndex];
                XMVECTOR interpolatedPosition = InterpolateFloat3Keys(prevKey, nextKey, timeStamp);
                return interpolatedPosition;
            }
        }
    }

    return DirectX::XMVECTOR();
}

DirectX::XMVECTOR AnimationChannel::InterpolateFloat3Keys(Float3Key& prevKey,
                                                          Float3Key& nextKey,
                                                          float timeStamp)
{
    const float range = nextKey.m_AnimationTime - prevKey.m_AnimationTime;
    const float pct = (timeStamp - prevKey.m_AnimationTime) / range;
    XMVECTOR v1 = XMLoadFloat3(&prevKey.m_KeyData);
    XMVECTOR v2 = XMLoadFloat3(&nextKey.m_KeyData);

    auto interpolatedResult = XMVectorLerp(v1, v2, pct);
    return interpolatedResult;
}