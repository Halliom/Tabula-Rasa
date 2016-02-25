#include "Player.h"

#include "glm\gtc\matrix_transform.hpp"

#include "../Engine/Input.h"

#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

Player::Player()
{
	Position = glm::vec3(0.0f, 0.0f, 0.0f);
	WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	Yaw = -90;
	Pitch = 0.0f;

	LastMouseX = 0;
	LastMouseY = 0;

	UpdateVectors();
}

Player::~Player()
{
}

void Player::Update(float DeltaTime)
{
	if (Input::Keys[KEY_W])
		Position += 2.0f * Front * DeltaTime;
	if (Input::Keys[KEY_S])
		Position -= 2.0f * Front * DeltaTime;
	if (Input::Keys[KEY_D])
		Position += 2.0f * Right * DeltaTime;
	if (Input::Keys[KEY_A])
		Position -= 2.0f * Right * DeltaTime;

	int32_t DeltaMouseX = Input::MouseX - LastMouseX;
	int32_t DeltaMouseY = Input::MouseY - LastMouseY;

	Yaw += DeltaMouseX * 25.0f * DeltaTime;
	Pitch += DeltaMouseY * -25.0f * DeltaTime;

	LastMouseX = Input::MouseX;
	LastMouseY = Input::MouseY;
	
	/*if (DeltaMouseX || DeltaMouseY)
		UpdateVectors();*/
}

void Player::UpdateVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	front.y = sin(glm::radians(this->Pitch));
	front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);
	this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}


glm::mat4 Player::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}
