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
	m_pLightPassShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexShaderLightPass.glsl"), 
		std::string("FragmentShaderLightPass.glsl"));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	TextRenderer::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();

	glGenVertexArrays(1, &m_ScreenQuadVAO);
	glGenBuffers(1, &m_ScreenQuadVBO);
	glGenBuffers(1, &m_ScreenQuadIBO);

	glBindVertexArray(m_ScreenQuadVAO);

	float ScreenQuad[] = { -1.0f, -1.0f,
							0.0f, 0.0f,	
							-1.0f, 1.0f,
							0.0f, 1.0f,
							1.0f, 1.0f,
							1.0f, 1.0f,
							1.0f, -1.0f,
							1.0f, 0.0f };

	GLubyte ScreenQuadIndices[] = { 0,1,2,   // first triangle (bottom left - top left - top right)
									0,2,3 }; // second triangle (bottom left - top right - bottom right)

	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, ScreenQuad, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuadIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, ScreenQuadIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) (sizeof(float) * 2));

	glBindVertexArray(0);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	glGenTextures(GBUFFER_LAYER_NUM, m_GBufferTextures);
	glGenTextures(1, &m_DepthTexture);

	for (int i = 0; i < GBUFFER_LAYER_NUM; ++i)
	{
		// Bind the texures one by one
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);

		// We only need RGB in the frame buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);

		// Nearest filtering since we want raw pixels not interpolations
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// This binds the texture of m_GBufferTextures[i] to a color attachment in the framebuffer
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
	}

#if 0
	// Do the same as above for the depth texture
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, ScreenWidth, ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
#endif

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
	StartGeometryPass();

	// Renders all chunks with greedy meshing technique
	ChunkRenderer::RenderAllChunks(RenderWorld->CurrentPlayer);

	EndGeometryPass();

	LightPass();

	// To text rendering last so it is on top of everything else
	TextRenderer::Render();
}

__forceinline void RenderingEngine::StartGeometryPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

__forceinline void RenderingEngine::EndGeometryPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void RenderingEngine::LightPass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pLightPassShader->Bind();

	GLsizei HalfWidth = (GLsizei)(m_ScreenWidth / 2.0f);
	GLsizei HalfHeight = (GLsizei)(m_ScreenHeight / 2.0f);

	for (int i = 0; i < GBUFFER_LAYER_NUM; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
	}

	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}

void RenderingEngine::ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	m_ScreenWidth = NewWidth;
	m_ScreenHeight = NewHeight;
}
