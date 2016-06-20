#pragma once

#include "glm\matrix.hpp"

#include "../Engine/Camera.h"

class Player
{
public:
	Player();

	~Player();

	void Update(float DeltaTime);

	void BeginPlay();

	void SetMovementSpeed(float NewSpeed);

	void SetPositionLua(float X, float Y, float Z)
	{
		m_pPlayerCamera->SetPosition(glm::vec3(X, Y, Z));
	}

	void SetPosition(glm::vec3 NewPosition)
	{
		m_pPlayerCamera->SetPosition(NewPosition);
	}

	Camera*			m_pPlayerCamera;
	class World*	m_pWorldObject;

private:

	float	m_Sensitivity;
	float	m_Yaw;
	float	m_Pitch;
	float	m_MovementSpeed;

	int		m_LastMouseX;
	int		m_LastMouseY;

};