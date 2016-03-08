#include "RenderingEngine.h"

#include "../Platform/Platform.h"
#include "../Game/Player.h"
#include "TextRenderer.h"
#include "ChunkRenderer.h"

#include "GL\glew.h"

extern Player* g_Player;

RenderingEngine::~RenderingEngine()
{
	UnloadFontLibrary();

	ChunkRenderer::DestroyChunkRenderer();
	TextRenderer::Destroy2DTextRendering();
}

void RenderingEngine::Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	TextRenderer::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	glGenTextures(GBUFFER_LAYER_NUM + 1, m_GBufferTextures);
	glGenTextures(1, &m_DepthTexture);

	for (int i = 0; i < GBUFFER_LAYER_NUM; ++i) 
	{
		// Bind the texures one by one
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);

		// We only need RGB in the frame buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);

		// This binds the texture of m_GBufferTextures[i] to a color attachment in the framebuffer
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
	}

	// Do the same as above for the depth texture
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, ScreenWidth, ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

	// Specify which buffers we want to draw to
	GLenum DrawBuffers[GBUFFER_LAYER_NUM];
	for (int i = 0; i < GBUFFER_LAYER_NUM; ++i)
	{
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glDrawBuffers(GBUFFER_LAYER_NUM, DrawBuffers);

	// Make sure no one else modifies this frame buffer (kinda like VAOs)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void RenderingEngine::RenderFrame(World* RenderWorld, const float& DeltaTime)
{
	// Renders all chunks with greedy meshing technique
	ChunkRenderer::RenderAllChunks(RenderWorld->CurrentPlayer);

	// To text rendering last so it is on top of everything else
	TextRenderer::Render();
}

__forceinline void RenderingEngine::StartRenderingIntoFrameBuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

__forceinline void RenderingEngine::StopRenderingIntoFrameBuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
}

void RenderingEngine::ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	m_ScreenWidth = NewWidth;
	m_ScreenHeight = NewHeight;
}
