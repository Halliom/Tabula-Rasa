#include "World.h"

#include "glm\common.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "../Rendering/ChunkRenderComponent.h"

World::World()
{
	CurrentPlayer = new Player();

	TextRender::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();

	LoadedChunks = new Chunk[4];
	NumLoadedChunks = 0;
}

World::~World()
{
	TextRender::Destroy2DTextRendering();

	ChunkRenderer::DestroyChunkRenderer();

	if (CurrentPlayer)
	{
		delete CurrentPlayer;
	}

	delete[] LoadedChunks;
}

void World::Update(float DeltaTime)
{
	static float Angle = 0;
	Angle += DeltaTime;

	ChunkRenderer::RenderAllChunks(CurrentPlayer, Angle);
	TextRender::Render();

	CurrentPlayer->Update(DeltaTime);
	
	for (unsigned int i = 0; i < NumLoadedChunks; ++i)
	{
		//TODO: Change this to be if the dirty flag is set then
		// update the render data and add to multirender draw
		LoadedChunks[i].Update();
	}
}
