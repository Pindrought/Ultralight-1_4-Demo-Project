#include "PCH.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	SetPosition({ 0, 0, 0 });
	InitializeProjectionValues(16.0f / 9.0f, 0.1f, 1000.0f);
	SetHorizontalFOV(DirectX::XM_PIDIV2);
}

void Camera::InitializeProjectionValues(float aspectRatio, float nearZ, float farZ)
{
	m_AspectRatio = aspectRatio;
	m_NearZ = nearZ;
	m_FarZ = farZ;
	UpdateProjectionMatrix();
}

void Camera::UpdateAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	UpdateProjectionMatrix();
}

float Camera::GetVerticalFOV()
{
	return m_FovY;
}

float Camera::GetHorizontalFOV()
{
	return m_FovX;
}

void Camera::SetVerticalFOV(float fovRadiansY)
{
	m_FovY = fovRadiansY;
	m_FovX = atan(tan(m_FovY / 2.0f) * m_AspectRatio) * 2;
	UpdateProjectionMatrix();
}

void Camera::SetHorizontalFOV(float fovRadiansX)
{
	m_FovX = fovRadiansX;
	m_FovY = atan(tan(m_FovX / 2.0f) * 1.0f / m_AspectRatio) * 2.0f;
	UpdateProjectionMatrix();
}

void Camera::SetPitchYawRoll(float pitch, float yaw, float roll)
{
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_Roll = roll;
	UpdateMatrix();
}

void Camera::SetPosition(Vector3 pos)
{
	m_Position = pos;
	UpdateMatrix();
}

const Matrix& Camera::GetViewMatrix() const
{
	return m_ViewMatrix;
}

const Matrix& Camera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}

const Matrix& Camera::GetInverseProjectionMatrix() const
{
	return m_InverseProjectionMatrix;
}

const Matrix& Camera::GetInverseViewMatrix() const
{
	return m_InverseViewMatrix;
}

const Matrix& Camera::GetInverseViewProjectionMatrix() const
{
	return m_InverseViewProjectionMatrix;
}

Scene* Camera::GetScene()
{
	return m_Scene.get();
}

void Camera::SetScene(shared_ptr<Scene> scene)
{
	m_Scene = scene;
}

void Camera::AdjustPitchYawRoll(float pitch, float yaw, float roll)
{
	m_Pitch += pitch;
	m_Yaw += yaw;
	while (m_Yaw > XM_2PI || m_Yaw < -XM_2PI)
	{
		m_Yaw = fmod(m_Yaw, XM_2PI);
	}
	m_Roll += roll;
	UpdateMatrix();
}

const Vector3 Camera::GetPitchYawRoll()
{
	Vector3 pyr(m_Pitch, m_Yaw, m_Roll);
	return pyr;
}

float Camera::GetNearZ()
{
	return m_NearZ;
}

float Camera::GetFarZ()
{
	return m_FarZ;
}

void Camera::UpdateProjectionMatrix()
{
	m_ProjectionMatrix = XMMatrixPerspectiveFovRH(m_FovY, m_AspectRatio, m_NearZ, m_FarZ);
	m_InverseProjectionMatrix = m_ProjectionMatrix.Invert();
	m_InverseViewProjectionMatrix = m_InverseProjectionMatrix * m_InverseViewMatrix;
}

void Camera::UpdateMatrix()
{
	//Calculate Camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll);

	//Calculate unit vector of cam target based off Camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = DirectX::XMVector3TransformCoord(Vector3::Forward, camRotationMatrix);
	//Calculate up direction based on current rotation
	XMVECTOR upDir = DirectX::XMVector3TransformCoord(Vector3::Up, camRotationMatrix);

	m_ViewMatrix = XMMatrixLookToRH(m_Position, camTarget, upDir);

	m_InverseViewMatrix = m_ViewMatrix.Invert();

	m_InverseViewProjectionMatrix = (m_ViewMatrix * m_ProjectionMatrix).Invert();
}

