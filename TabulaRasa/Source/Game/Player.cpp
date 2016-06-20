#include "Player.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Input.h"
#include "../Rendering/GuiSystem.h"

#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

Player* g_Player = NULL;

Player::Player() :
	m_Sensitivity(10.0f),
	m_MovementSpeed(4.0f),
	m_Yaw(-90.0f),
	m_Pitch(0.0f),
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
	if (!Input::IsGameFrozen)
	{
		m_pPlayerCamera->m_OldPosition = m_pPlayerCamera->m_Position;
		bool UpdatedMovement = false;
		if (Input::Keys[SDL_SCANCODE_W])
		{
			m_pPlayerCamera->m_Position += m_MovementSpeed * m_pPlayerCamera->m_Front * DeltaTime;
			UpdatedMovement = true;
		}
		if (Input::Keys[SDL_SCANCODE_S])
		{
			m_pPlayerCamera->m_Position -= m_MovementSpeed * m_pPlayerCamera->m_Front * DeltaTime;
			UpdatedMovement = true;
		}
		if (Input::Keys[SDL_SCANCODE_D])
		{
			m_pPlayerCamera->m_Position += m_MovementSpeed * m_pPlayerCamera->m_Right * DeltaTime;
			UpdatedMovement = true;
		}
		if (Input::Keys[SDL_SCANCODE_A])
		{
			m_pPlayerCamera->m_Position -= m_MovementSpeed * m_pPlayerCamera->m_Right * DeltaTime;
			UpdatedMovement = true;
		}

		m_pPlayerCamera->m_bIsViewMatrixDirty = UpdatedMovement;

		int DeltaMouseX = Input::MouseX - m_LastMouseX;
		int DeltaMouseY = Input::MouseY - m_LastMouseY;

		m_Yaw += (float)DeltaMouseX * m_Sensitivity * DeltaTime;
		m_Pitch += (float)DeltaMouseY * -m_Sensitivity * DeltaTime;

		m_Pitch = glm::max(-90.0f, m_Pitch);

		m_LastMouseX = Input::MouseX;
		m_LastMouseY = Input::MouseY;

		if (DeltaMouseX != 0.0f || DeltaMouseY != 0.0f)
			m_pPlayerCamera->UpdateCameraRotation(m_Yaw, m_Pitch);
	}
}

void Player::BeginPlay()
{
	// Init the projection
	m_pPlayerCamera->InitProjection(70.0f);

	// Set the global instance
	g_Player = this;
}

void Player::SetMovementSpeed(float NewSpeed)
{
	assert(NewSpeed > 0.0f);
	m_MovementSpeed = NewSpeed;
}
