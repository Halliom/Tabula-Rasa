#pragma once

#include "glm\matrix.hpp"

class Player
{
public:
	Player();

	~Player();

	void Update(float DeltaTime);

	void UpdateVectors();

	glm::mat4 GetViewMatrix();

private:

	glm::vec3 Position;

	glm::vec3 Front;

	glm::vec3 Up;

	glm::vec3 Right;

	glm::vec3 WorldUp;

	float Yaw;

	float Pitch;

	int32_t LastMouseX;

	int32_t LastMouseY;

};