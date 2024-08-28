#pragma once
#include "PCH.h"
#include "Model.h"
#include "../EZGLTF/SkeletonPose.h"

class Entity
{
public:
	void AdvanceAnimation(float deltaTime);
	EZGLTF::AnimationClip* GetActiveAnimationClip();
	void SetAnimationClip(EZGLTF::AnimationClip* clip);
	Matrix GetMatrix();
	Model* GetModel();
	Vector3 GetPosition();
	Quaternion GetRotation();
	Vector3 GetScale();
	bool IsVisible() const;
	void SetAnimationSpeed(float animationSpeed);
	void SetModel(shared_ptr<Model> model);
	void SetPosition(Vector3 pos);
	void SetRotation(Quaternion rot);
	void SetScale(Vector3 scale);
	void SetVisibility(bool isVisible);
	void UpdatePose();
	EZGLTF::SkeletonPose& GetPose();
private:
	void UpdateMatrix();
	EZGLTF::SkeletonPose m_SkeletonPose;
	EZGLTF::AnimationClip* m_ActiveAnimation = nullptr;
	float m_AnimationTime = 0;
	float m_AnimationSpeed = 1;
	shared_ptr<Model> m_Model = nullptr;
	Quaternion m_Quaternion = Quaternion::Identity; //Rotation
	Vector3 m_Scale = Vector3(1, 1, 1);
	bool m_IsVisible = true;
	Vector3 m_Position = { 0, 0, 0 };
	Matrix m_WorldMatrix;
};