#pragma once
#include "PCH.h"

namespace EZGLTF
{
	struct FloatKey
	{
		FloatKey()
		{
			m_AnimationTime = 0;
			m_KeyData = 0;
		}
		FloatKey(float keyData, float animationTime)
		{
			m_KeyData = keyData;
			m_AnimationTime = animationTime;
		}
		float m_AnimationTime = 0;
		float m_KeyData;
	};

	struct Float3Key
	{
		Float3Key()
		{
			m_KeyData = DirectX::XMFLOAT3();
			m_AnimationTime = 0;
		}
		Float3Key(DirectX::XMFLOAT3 keyData, float animationTime)
		{
			m_KeyData = keyData;
			m_AnimationTime = animationTime;
		}
		float m_AnimationTime = 0;
		DirectX::XMFLOAT3 m_KeyData;
	};

	struct Float4Key
	{
		Float4Key()
		{
			m_KeyData = DirectX::XMFLOAT4();
			m_AnimationTime = 0;
		}
		Float4Key(DirectX::XMFLOAT4 keyData, float animationTime)
		{
			m_KeyData = keyData;
			m_AnimationTime = animationTime;
		}
		float m_AnimationTime = 0;
		DirectX::XMFLOAT4 m_KeyData; //rotation expressed as a quaternion
	};
}