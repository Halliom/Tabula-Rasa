#include "Player.h"

#include "glm/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "../Engine/Input.h"
#include "../Engine/Camera.h"
#include "../Rendering/GuiSystem.h"
#include "../Game/World.h"

#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

Player* g_Player = NULL;

Player::Player() :
	m_Yaw(-90.0f),
	m_Pitch(0.0f),
    m_Position(glm::vec3(0.0f, 0.0f, 0.0f)),
	m_OldPosition(glm::vec3(0.0f, 0.0f, 0.0f)),
    m_MovementSpeed(4.0f),
    m_Sensitivity(10.0f),
    m_LastMouseX(0),
    m_LastMouseY(0)
{
	m_pPlayerCamera = new Camera();
}

Player::~Player()
{
	// Remove from global instance
	g_Player = NULL;

	delete m_pPlayerCamera;
}

void Player::Update(float DeltaTime)
{
	if (Input::Keys[SDL_SCANCODE_W])
	{
		AddMovementInput(MID_FORWARD, DeltaTime);
	}
	if (Input::Keys[SDL_SCANCODE_S])
	{
		AddMovementInput(MID_BACKWARD, DeltaTime);
	}
	if (Input::Keys[SDL_SCANCODE_D])
	{
		AddMovementInput(MID_RIGHT, DeltaTime);
	}
	if (Input::Keys[SDL_SCANCODE_A])
	{
		AddMovementInput(MID_LEFT, DeltaTime);
	}

	// Add the yaw fromt the mouse (x) delta
	AddYawInput((float)(Input::MouseX - m_LastMouseX) * m_Sensitivity * DeltaTime);

	// Add the pitch from the mouse (y) delta
	AddPitchInput((float)(Input::MouseY - m_LastMouseY) * -m_Sensitivity * DeltaTime);

	// Save the previous state
	m_LastMouseX = Input::MouseX;
	m_LastMouseY = Input::MouseY;

	static bool Previous = false;
	if (!Input::IsGameFrozen && Input::MouseButtons[1] && !Previous)
	{
		m_pWorldObject->RayTraceWorld(m_pPlayerCamera->GetViewingRay());
	}
	Previous = Input::MouseButtons[1];
}

void Player::BeginPlay()
{
	// Init the projection
	m_pPlayerCamera->InitProjection();

	// Set the global instance
	g_Player = this;
}

void Player::SetMovementSpeed(float NewSpeed)
{
	assert(NewSpeed > 0.0f);
	m_MovementSpeed = NewSpeed;
}

// TODO: Make it so that no input will ever reach this if the game is frozen, and 
// remove the "if (!Input::IsGameFrozen)" part
void Player::AddMovementInput(const EMovementInputDirection& Direction, float Value)
{
	if (!Input::IsGameFrozen)
	{
		m_OldPosition = m_Position;
		switch (Direction)
		{
			case MID_FORWARD:
			{
				m_Position += m_MovementSpeed * m_pPlayerCamera->m_Front * Value;
				break;
			}
			case MID_BACKWARD:
			{
				m_Position -= m_MovementSpeed * m_pPlayerCamera->m_Front * Value;
				break;
			}
			case MID_RIGHT:
			{
				m_Position += m_MovementSpeed * m_pPlayerCamera->m_Right * Value;
				break;
			}
			case MID_LEFT:
			{
				m_Position -= m_MovementSpeed * m_pPlayerCamera->m_Right * Value;
				break;
			}
		}
		m_pPlayerCamera->UpdatePosition(m_Position);
	}
}

void Player::AddYawInput(float Value)
{
	if (Value && !Input::IsGameFrozen)
	{
		// Add the value to the players yaw
		m_Yaw += Value;

		// Update the camera
		m_pPlayerCamera->UpdateCameraRotation(m_Yaw, m_Pitch);
	}
}

void Player::AddPitchInput(float Value)
{
	if (Value && !Input::IsGameFrozen)
	{
		// Add the value and make sure that it doesn't go to 90 degrees (snap the players neck)
		m_Pitch += Value;
		m_Pitch = glm::max(-89.99f, glm::min(m_Pitch, 89.99f));

		// Update the camera
		m_pPlayerCamera->UpdateCameraRotation(m_Yaw, m_Pitch);
	}
}

void Player::SetPosition(glm::vec3 NewPosition)
{
	// Update the cameras position
	m_pPlayerCamera->UpdatePosition(NewPosition);

	// Save the old position and update with the new position
	m_OldPosition = m_Position;
	m_Position = NewPosition;
}
