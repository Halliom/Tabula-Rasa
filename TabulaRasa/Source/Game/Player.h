#pragma once

#include "glm\matrix.hpp"

enum EMovementInputDirection
{
	MID_FORWARD,
	MID_BACKWARD,
	MID_RIGHT,
	MID_LEFT
};

class Player
{
public:
	Player();

	~Player();

	void BeginPlay();

	void Update(float DeltaTime);

	void SetMovementSpeed(float NewSpeed);

	void AddMovementInput(const EMovementInputDirection& Direction, float Value);

	void AddYawInput(float Value);

	void AddPitchInput(float Value);

	glm::vec3 GetPosition() const 
	{
		return m_Position;
	}

	void SetPositionLua(float X, float Y, float Z)
	{
		SetPosition(glm::vec3(X, Y, Z));
	}

	void SetPosition(glm::vec3 NewPosition);

	class Camera*	m_pPlayerCamera;
	class World*	m_pWorldObject;

	float			m_Yaw;
	float			m_Pitch;

	glm::vec3		m_Position;
	glm::vec3		m_OldPosition;
	float			m_MovementSpeed;

private:

	float	m_Sensitivity;

	int		m_LastMouseX;
	int		m_LastMouseY;

};