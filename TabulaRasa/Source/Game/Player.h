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

	Camera* PlayerCamera;

private:

	float Yaw;

	float Pitch;

	int32_t LastMouseX;

	int32_t LastMouseY;

};