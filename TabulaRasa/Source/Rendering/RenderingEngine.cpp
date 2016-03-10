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

	glDeleteFramebuffers(1, &m_FBO);
	glDeleteTextures(GBUFFER_LAYER_NUM, m_GBufferTextures);
	glDeleteTextures(1, &m_DepthTexture);

	glDeleteVertexArrays(1, &m_ScreenQuadVAO);
	glDeleteBuffers(1, &m_ScreenQuadVBO);
	glDeleteBuffers(1, &m_ScreenQuadIBO);
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

	SetupQuad();

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glGenTextures(GBUFFER_LAYER_NUM, m_GBufferTextures);
	glGenTextures(1, &m_DepthTexture);

	// Bind the texures one by one
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_POSITION]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// This binds the texture of m_GBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_POSITION], 0);

	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_NORMAL]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// This binds the texture of m_GBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_NORMAL], 0);

	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_TEXCOORD]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// This binds the texture of m_GBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_TEXCOORD], 0);

	// Do the same as above for the depth texture
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, ScreenWidth, ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

	// Specify which buffers we want to draw to
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(GBUFFER_LAYER_NUM, DrawBuffers);

	// Make sure no one else modifies this frame buffer (kinda like VAOs)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::SetupQuad()
{
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
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	glBindVertexArray(0);
}

void RenderingEngine::RenderFrame(World* RenderWorld, const float& DeltaTime)
{
	StartGeometryPass();

	// Renders all chunks with greedy meshing technique
	ChunkRenderer::RenderAllChunks(RenderWorld->CurrentPlayer);

	LightPass();

	// To text rendering last so it is on top of everything else
	TextRenderer::Render();
}

__forceinline void RenderingEngine::StartGeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingEngine::LightPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pLightPassShader->Bind();
	m_pLightPassShader->SetDefaultSamplers();
	
	// Bind the Position texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_POSITION]);

	// Bind the normal texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_NORMAL]);

	// Bind the texture coord texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GBUFFER_LAYER_TEXCOORD]);

	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}

void RenderingEngine::ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	m_ScreenWidth = NewWidth;
	m_ScreenHeight = NewHeight;
}
