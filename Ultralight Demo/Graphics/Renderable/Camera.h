#pragma once
#include "PCH.h"
#include "Scene.h"

class Camera
{
public:
	Camera();
	void InitializeProjectionValues(float aspectRatio, float nearZ, float farZ);
	void UpdateAspectRatio(float aspectRatio);
	float GetVerticalFOV();
	float GetHorizontalFOV();
	void SetVerticalFOV(float fovRadiansY);
	void SetHorizontalFOV(float fovRadiansX);
	void SetPitchYawRoll(float pitch, float yaw, float roll);
	void SetPosition(Vector3 pos);
	const Matrix& GetViewMatrix() const;
	const Matrix& GetProjectionMatrix() const;
	const Matrix& GetInverseProjectionMatrix() const;
	const Matrix& GetInverseViewMatrix() const;
	const Matrix& GetInverseViewProjectionMatrix() const;
	Scene* GetScene();
	void SetScene(shared_ptr<Scene> scene);
	void AdjustPitchYawRoll(float pitch, float yaw, float roll);

	const Vector3 GetPitchYawRoll();

	float GetNearZ();
	float GetFarZ();
private:
	void UpdateMatrix();
	void UpdateProjectionMatrix();
	Matrix m_ViewMatrix;
	Matrix m_ProjectionMatrix;
	Matrix m_InverseViewMatrix;
	Matrix m_InverseProjectionMatrix;
	Matrix m_InverseViewProjectionMatrix;
	Vector3 m_Position;

	float m_NearZ = 0.1f;
	float m_FarZ = 1000.0f;
	float m_FovX = DirectX::XM_PIDIV2;
	float m_FovY = DirectX::XM_PIDIV2;
	float m_AspectRatio = 4.0f / 3.0f;
	float m_Pitch = 0;
	float m_Yaw = 0;
	float m_Roll = 0;
	shared_ptr<Scene> m_Scene = nullptr;
};