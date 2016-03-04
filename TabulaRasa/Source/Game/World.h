#pragma once

#include "../Rendering/GL_shader.h"

#include "../Rendering/TextRender.h"

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

	Chunk* LoadedChunks;

	unsigned int NumLoadedChunks;

	GLShaderProgram* Shader;
};