#include "RenderingEngine.h"

#include "../Platform/Platform.h"
#include "../Game/Player.h"
#include "TextRender.h"
#include "ChunkRenderComponent.h"

extern Player* g_Player;

RenderingEngine::~RenderingEngine()
{
	UnloadFontLibrary();

	ChunkRenderer::DestroyChunkRenderer();
	TextRender::Destroy2DTextRendering();
}

void RenderingEngine::Initialize(const bool& UseDepthTest)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	if (UseDepthTest)
	{
		glEnable(GL_DEPTH_TEST);
	}

	TextRender::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();
}

void RenderingEngine::RenderFrame(World* RenderWorld, const float & DeltaTime)
{
	ChunkRenderer::RenderAllChunks(RenderWorld->CurrentPlayer);

	// To text rendering last so it is on top of everything else
	TextRender::Render();
}
