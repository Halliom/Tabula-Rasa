#pragma once

#include "../Rendering/GL_shader.h"

#include "../Engine/Octree.h"
#include "Player.h"

class World
{
public:
	World();

	~World();

	void Update(float DeltaTime);

private:

	Player* CurrentPlayer;

	Chunk Chunk;

	GLShaderProgram* Shader;
};