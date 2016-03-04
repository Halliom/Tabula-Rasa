#include "Camera.h"

#include "../Platform/Platform.h"

Camera* Camera::ActiveCamera = NULL;

void Camera::InitProjection(const float& FOV, const float& NearPlane, const float& FarPlane)
{
	WindowWidth = PlatformWindow::GlobalWindow->WindowParams.Width;
	WindowHeight = PlatformWindow::GlobalWindow->WindowParams.Height;

	ProjectionMatrix = glm::perspective(FOV, (float) WindowWidth / (float) WindowHeight, NearPlane, FarPlane);

	IsScreenMatrixDirty = true;
	IsViewMatrixDirty = true;

	ActiveCamera = this;
}

void Camera::UpdateCameraRotation(const float & Yaw, const float & Pitch)
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	this->Front = glm::normalize(front);
	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));

	IsViewMatrixDirty = true;
}
