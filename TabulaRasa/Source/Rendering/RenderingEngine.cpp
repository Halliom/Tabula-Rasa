#include "RenderingEngine.h"

#include <random>
#include "glm\common.hpp"

#include "../Platform/Platform.h"
#include "../Game/Player.h"
#include "TextRenderer.h"
#include "ChunkRenderer.h"
#include "../Game/World.h"

#include "../Engine/Console.h"

#include "GL\glew.h"

extern Player* g_Player;

RenderingEngine::~RenderingEngine()
{
	ChunkRenderer::DestroyChunkRenderer();
	TextRenderer::Destroy2DTextRendering();

	glDeleteFramebuffers(1, &m_GeometryFBO);
	glDeleteTextures(GBUFFER_LAYER_GEOMETRY_NUM, m_GeometryGBufferTextures);
	glDeleteTextures(1, &m_GeometryDepthTexture);

	glDeleteVertexArrays(1, &m_ScreenQuadVAO);
	glDeleteBuffers(1, &m_ScreenQuadVBO);
	glDeleteBuffers(1, &m_ScreenQuadIBO);
}

void RenderingEngine::Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight)
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;

	m_pLightPassShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderLightPass.glsl"));

	glViewport(0, 0, ScreenWidth, ScreenHeight);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	TextRenderer::Initialize2DTextRendering();
	ChunkRenderer::SetupChunkRenderer();

	SetupQuad();

	SetupGeometryPass();

	SetupSSAOPass();
}

void RenderingEngine::PostInitialize()
{
	LogLn("Rendering system initialized on system:");
	LogLn((char*) glGetString(GL_VENDOR));
	LogLn((char*) glGetString(GL_RENDERER));
	LogLn((char*) glGetString(GL_VERSION));

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	char Buffer[32];
	sprintf(Buffer, "Number of processors: %d\n", SystemInfo.dwNumberOfProcessors);
	Log(Buffer);
}

void RenderingEngine::AddRendererForBlock(unsigned int BlockID, const char *BlockModelFileName)
{
	PlatformFileSystem::LoadModel(&CustomBlockRenderers[BlockID], BlockModelFileName);
}

void RenderingEngine::SetupGeometryPass()
{
	glGenFramebuffers(1, &m_GeometryFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryFBO);

	glGenTextures(GBUFFER_LAYER_GEOMETRY_NUM, m_GeometryGBufferTextures);
	glGenTextures(1, &m_GeometryDepthTexture);

	// Bind the texures one by one
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_POSITION]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_ScreenWidth, m_ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_POSITION], 0);

	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_NORMAL]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_NORMAL], 0);

	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_TEXCOORD]);
	// We only need RGB in the frame buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_TEXCOORD], 0);

	// Do the same as above for the depth texture
	glBindTexture(GL_TEXTURE_2D, m_GeometryDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_ScreenWidth, m_ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GeometryDepthTexture, 0);

	// Specify which buffers we want to draw to
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(3, DrawBuffers);

	// Make sure no one else modifies this frame buffer (kinda like VAOs)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::SetupSSAOPass()
{
	m_pSSAOShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderSSAO.glsl"),
		true);

	glGenFramebuffers(1, &m_SSAOFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

	glGenTextures(1, &m_SSAOColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);
	// We only need one value (since SSAO is in grayscale) so we tell it to store it in the red component
	// since red is the first component in a color vector
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOColorBuffer, 0);

	// Make sure no one else modifies it
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generate noise for the SSAO pass
	std::uniform_real_distribution<GLfloat> FloatRange(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine RandomGenerator;
	float SSAONoise[48]; // 16 vector3s
    for (unsigned int i = 0; i < 48; i += 3)
    {
		SSAONoise[i]		= FloatRange(RandomGenerator) * 2.0 - 1.0;
		SSAONoise[i + 1]	= FloatRange(RandomGenerator) * 2.0 - 1.0;
		SSAONoise[i + 2]	= 0.0f;
    }

	glGenTextures(1, &m_SSAONoiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, SSAONoise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Needs to be repeat so that the noise will tile across the screen
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Generate kernels
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 Sample = glm::vec3(
			FloatRange(RandomGenerator) * 2.0 - 1.0,
			FloatRange(RandomGenerator) * 2.0 - 1.0,
			FloatRange(RandomGenerator));
		Sample = glm::normalize(Sample);
		Sample *= FloatRange(RandomGenerator);

		// Scale samples s.t. they're more aligned to center of kernel
		float Scale = (float)i / 64.0f;

		// lerp(a, b, f) (lerp(0.1f, 1.0f, Scale * Scale))
		// a + f * (b - a)
        Scale = 0.1f + (Scale * Scale) * (1.0f - 0.1f);
        Sample *= Scale;
		m_pSSAOKernel[(i * 3)]		= Sample.x;
		m_pSSAOKernel[(i * 3) + 1]	= Sample.y;
		m_pSSAOKernel[(i * 3) + 2]	= Sample.z;
	}

	/**
	 * Setup the blur pass
	 */

	m_pSSAOBlurShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderSSAOBlur.glsl"));

	glGenFramebuffers(1, &m_SSAOBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);

	glGenTextures(1, &m_SSAOBlurColorBuffer);
	glBindTexture(GL_TEXTURE_2D, m_SSAOBlurColorBuffer);
	// We only need one value (since SSAO is in grayscale) so we tell it to store it in the red component
	// since red is the first component in a color vector
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_ScreenWidth, m_ScreenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	// Nearest filtering since we want raw pixels not interpolations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOBlurColorBuffer, 0);

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
	// Renders all the chunks in the geometry pass
	StartGeometryPass();
	ChunkRenderer::RenderAllChunks(RenderWorld->CurrentPlayer);

	SSAOPass();

	LightPass();

	// To text rendering last so it is on top of everything else
	TextRenderer::Render();
}

void RenderingEngine::SSAOPass()
{
	// Do the SSAO Pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	m_pSSAOShader->Bind();
	m_pSSAOShader->SetDefaultSamplers();
	m_pSSAOShader->SetProjectionMatrix(*Camera::ActiveCamera->GetProjectionMatrix());
	m_pSSAOShader->SetScreenDimension(glm::vec2(m_ScreenWidth, m_ScreenHeight));

	// Bind the textures from the gemoetry pass
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_POSITION]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_NORMAL]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_SSAONoiseTexture);
	m_pSSAOShader->SetSSAOSamples(m_pSSAOKernel);

	// Render a screen quad
	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);

	/**
	 * Do blur pass
	 */

	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	m_pSSAOBlurShader->Bind();
	m_pSSAOBlurShader->SetDefaultSamplers();

	// Send the texture fromt he previous SSAO pass to the shader to get blurred
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_SSAOColorBuffer);

	// Render a screen quad
	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

__forceinline void RenderingEngine::StartGeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryFBO);
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
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_POSITION]);

	// Bind the normal texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_NORMAL]);

	// Bind the texture coord texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_TEXCOORD]);

	// Bind the texture atlas for the block textures
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_SSAOBlurColorBuffer);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ChunkRenderer::g_TextureAtlas);

	// Render a screen quad
	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);
}

void RenderingEngine::ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	m_ScreenWidth = NewWidth;
	m_ScreenHeight = NewHeight;
	glViewport(0, 0, NewWidth, NewHeight);
}
