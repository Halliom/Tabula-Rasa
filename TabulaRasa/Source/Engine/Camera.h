#pragma once

#include "glm/common.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct Ray
{
	glm::vec3 Origin;
	glm::vec3 Direction;
	float Distance;
};

class Camera
{
public:

	Camera();

	void InitProjection(const float& FOV = 70.0f, const float& NearPlane = 0.1f, const float& FarPlane = 160.0f);

	void UpdateCameraRotation(const float& Yaw, const float& Pitch);

	void UpdatePosition(glm::vec3 NewPosition);

	Ray GetViewingRay(float Distance = 100000.0f);
	
	__forceinline glm::mat4* GetProjectionMatrix()
	{
		if (m_bIsProjectionMatrixDirty)
		{
			m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), (float)m_WindowWidth / (float)m_WindowHeight, m_NearPlane, m_FarPlane);
			m_bIsProjectionMatrixDirty = false;
		}
		return &m_ProjectionMatrix;
	}

	__forceinline glm::mat4* GetScreenMatrix()
	{
		if (m_bIsScreenMatrixDirty)
		{
			m_ScreenMatrix = glm::ortho(0.0f, (float) m_WindowWidth, (float) m_WindowHeight, 0.0f);
			m_bIsScreenMatrixDirty = false;
		}
		return &m_ScreenMatrix;
	}

	__forceinline glm::mat4* GetViewMatrix()
	{
		if (m_bIsViewMatrixDirty)
		{
			m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
			m_bIsViewMatrixDirty = false;
		}
		return &m_ViewMatrix;
	}

	// The static camera that is active for this client
	static Camera*	g_ActiveCamera;

	unsigned int	m_WindowWidth;
	unsigned int	m_WindowHeight;
	bool			m_bIsProjectionMatrixDirty;
	bool			m_bIsScreenMatrixDirty;
	bool			m_bIsViewMatrixDirty;

	float			m_FOV;
	float			m_NearPlane;
	float			m_FarPlane;

	glm::vec3		m_Position;
	glm::vec3		m_OldPosition;
	glm::vec3		m_Front;
	glm::vec3		m_Up;
	glm::vec3		m_Right;
	glm::vec3		m_WorldUp;

private:

	glm::mat4		m_ProjectionMatrix;
	glm::mat4		m_ScreenMatrix;
	glm::mat4		m_ViewMatrix;
};