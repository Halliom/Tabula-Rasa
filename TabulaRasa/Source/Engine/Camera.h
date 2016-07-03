#pragma once

#include "glm/common.hpp"
#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "../Engine/Engine.h"

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
    
    void UpdateScreenDimensions(unsigned int NewWidth, unsigned int NewHeight);
	
	FORCEINLINE void SetProjectionMatrixDirty()
	{
        m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), (float)m_WindowWidth / (float)m_WindowHeight, m_NearPlane, m_FarPlane);
	}

	FORCEINLINE void SetScreenMatrixDirty()
	{
        m_ScreenMatrix = glm::ortho(0.0f, (float) m_WindowWidth, (float) m_WindowHeight, 0.0f);
	}

	FORCEINLINE void SetViewMatrixDirty()
	{
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	}

	// The static camera that is active for this client
	static Camera*	g_ActiveCamera;

	glm::vec3		m_Position;
	glm::vec3		m_OldPosition;
	glm::vec3		m_Front;
	glm::vec3		m_Up;
	glm::vec3		m_Right;
	glm::vec3		m_WorldUp;
    
    glm::mat4		m_ProjectionMatrix;
    glm::mat4		m_ScreenMatrix;
    glm::mat4		m_ViewMatrix;
    
private:
    
    unsigned int	m_WindowWidth;
    unsigned int	m_WindowHeight;
    
    float			m_FOV;
    float			m_NearPlane;
    float			m_FarPlane;
};