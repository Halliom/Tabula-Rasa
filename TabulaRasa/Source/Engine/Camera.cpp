#include "Camera.h"

#include "../Platform/Platform.h"

Camera* Camera::g_ActiveCamera = NULL;

Camera::Camera() :
	m_Position(glm::vec3(0.0f, 0.0f, 0.0f)),
	m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	m_WindowWidth(PlatformWindow::GlobalWindow->WindowParams.Width), // Get startup value for width
	m_WindowHeight(PlatformWindow::GlobalWindow->WindowParams.Height) // Get startup value for height
{
	UpdateCameraRotation(-90.0f, 0.0f);
}

void Camera::InitProjection(const float& FOV, const float& NearPlane, const float& FarPlane)
{
	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), (float) m_WindowWidth / (float) m_WindowHeight, NearPlane, FarPlane);

	m_FOV = FOV;
	m_NearPlane = NearPlane;
	m_FarPlane = FarPlane;

	m_bIsScreenMatrixDirty = true;
	m_bIsViewMatrixDirty = true;

	g_ActiveCamera = this;
}

void Camera::UpdateCameraRotation(const float& Yaw, const float& Pitch)
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	this->m_Front = glm::normalize(front);
	this->m_Right = glm::normalize(glm::cross(this->m_Front, this->m_WorldUp));
	this->m_Up = glm::normalize(glm::cross(this->m_Right, this->m_Front));

	m_bIsViewMatrixDirty = true;
}
