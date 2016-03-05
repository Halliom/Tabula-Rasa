#pragma once

#include "glm\common.hpp"
#include "glm\matrix.hpp"
#include "glm\gtc\matrix_transform.hpp"

class Camera
{
public:

	Camera();

	void InitProjection(const float& FOV = 45.0f, const float& NearPlane = 0.1f, const float& FarPlane = 100.0f);

	void UpdateCameraRotation(const float& Yaw, const float& Pitch);

	__forceinline glm::mat4* GetProjectionMatrix()
	{
		if (IsProjectionMatrixDirty)
		{
			ProjectionMatrix = glm::perspective(glm::radians(FOV), (float) WindowWidth / (float) WindowHeight, 0.01f, 100.0f);
			IsProjectionMatrixDirty = false;
		}
		return &ProjectionMatrix;
	}

	__forceinline glm::mat4* GetScreenMatrix()
	{
		if (IsScreenMatrixDirty)
		{
			ScreenMatrix = glm::ortho(0.0f, (float) WindowWidth, (float) WindowHeight, 0.0f);
			IsScreenMatrixDirty = false;
		}
		return &ScreenMatrix;
	}

	__forceinline glm::mat4* GetViewMatrix()
	{
		if (IsViewMatrixDirty)
		{
			ViewMatrix = glm::lookAt(Position, Position + Front, Up);
			IsViewMatrixDirty = false;
		}
		return &ViewMatrix;
	}

	static Camera* ActiveCamera;

	unsigned int WindowWidth;

	unsigned int WindowHeight;

	float FOV;

	bool IsProjectionMatrixDirty;

	bool IsScreenMatrixDirty;

	bool IsViewMatrixDirty;

	glm::vec3 Position;

	glm::vec3 Front;

	glm::vec3 Up;

	glm::vec3 Right;

	glm::vec3 WorldUp;

private:

	glm::mat4 ProjectionMatrix;

	glm::mat4 ScreenMatrix;
	
	glm::mat4 ViewMatrix;
};