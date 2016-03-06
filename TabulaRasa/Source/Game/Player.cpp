#include "Player.h"

#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Input.h"

#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

Player* g_Player = NULL;

Player::Player()
{
	PlayerCamera = new Camera();
	Yaw = -90;
	Pitch = 0.0f;

	LastMouseX = 0;
	LastMouseY = 0;
}

Player::~Player()
{
	// Remove from global instance
	g_Player = NULL;

	delete PlayerCamera;
}

#include "../Rendering/TextRenderer.h"

void Player::Update(float DeltaTime)
{
	bool UpdatedMovement = false;
	if (Input::Keys[SDL_SCANCODE_W])
	{
		PlayerCamera->Position += 2.0f * PlayerCamera->Front * DeltaTime;
		UpdatedMovement = true;
	}
	if (Input::Keys[SDL_SCANCODE_S])
	{
		PlayerCamera->Position -= 2.0f * PlayerCamera->Front * DeltaTime;
		UpdatedMovement = true;
	}
	if (Input::Keys[SDL_SCANCODE_D])
	{
		PlayerCamera->Position += 2.0f * PlayerCamera->Right * DeltaTime;
		UpdatedMovement = true;
	}
	if (Input::Keys[SDL_SCANCODE_A])
	{
		PlayerCamera->Position -= 2.0f * PlayerCamera->Right * DeltaTime;
		UpdatedMovement = true;
	}

	PlayerCamera->IsViewMatrixDirty = UpdatedMovement;

	int DeltaMouseX = Input::MouseX - LastMouseX;
	int DeltaMouseY = Input::MouseY - LastMouseY;

	Yaw += (float) DeltaMouseX * 100.0f * DeltaTime;
	Pitch += (float) DeltaMouseY * -100.0f * DeltaTime;

	LastMouseX = Input::MouseX;
	LastMouseY = Input::MouseY;

	//if (DeltaMouseX || DeltaMouseY)
	PlayerCamera->UpdateCameraRotation(Yaw, Pitch);
}

void Player::BeginPlay()
{
	// Init the projection
	PlayerCamera->InitProjection();

	// Set the global instance
	g_Player = this;
}
