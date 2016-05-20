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

	Camera* m_pPlayerCamera;

	class World* m_pWorldObject;

private:

	float m_Sensitivity;
	float m_MovementSpeed;

	float m_Yaw;
	float m_Pitch;

	int m_LastMouseX;
	int m_LastMouseY;

};