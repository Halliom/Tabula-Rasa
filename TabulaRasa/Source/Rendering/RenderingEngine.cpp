#include "RenderingEngine.h"

#include <random>
#include "glm/common.hpp"

#include "../Platform/Platform.h"

#include "../Game/World.h"
#include "../Game/Player.h"
#include "../Engine/Console.h"
#include "../Engine/Camera.h"
#include "../Engine/Core/List.h"
#include "GuiSystem.h"
#include "ChunkRenderer.h"

#include "GL/glew.h"

RenderingEngine::~RenderingEngine()
{
	DeleteFrameBuffers();

    // Delete the Quad (for rendering to screen
	glDeleteVertexArrays(1, &m_ScreenQuadVAO);
	glDeleteBuffers(1, &m_ScreenQuadVBO);
	glDeleteBuffers(1, &m_ScreenQuadIBO);
    
    // Delete the shaders
    delete m_pLightPassShader;
    delete m_pSSAOShader;
    delete m_pSSAOBlurShader;
    
    delete m_pChunkRenderer;
}

void RenderingEngine::Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight)
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;

	// SSAO shader
	m_pSSAOShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderSSAO.glsl"),
		true);

	// SSAO blur shader
	m_pSSAOBlurShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderSSAOBlur.glsl"));

	// Light pass shader
	m_pLightPassShader = GLShaderProgram::CreateVertexFragmentShaderFromFile(
		std::string("VertexPassthrough.glsl"),
		std::string("FragmentShaderLightPass.glsl"));

	glViewport(0, 0, ScreenWidth, ScreenHeight);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
    
    // Instantiate and initialize the chunk renderer
    m_pChunkRenderer = new ChunkRenderer();
	m_pChunkRenderer->SetupChunkRenderer();
    
    // Load the texture atlas for all the blocks textures'
    unsigned int Width, Height;
    std::string FileName = PlatformFileSystem::GetAssetDirectory(DT_TEXTURES).append(std::string("textures.png"));
	m_TextureAtlas = PlatformFileSystem::LoadImageFromFile((char*)FileName.c_str(), Width, Height);

	// It will later be used as texture 4
	m_TextureAtlas.m_BindingPoint = 4;
    
	SetupQuad();

	SetupGeometryPass();

	SetupSSAOPass();
}

void RenderingEngine::PostInitialize()
{
	Log("Rendering system initialized on system:");
	LogF("Vendor: %s", (char*) glGetString(GL_VENDOR));
	LogF("Renderer: %s", (char*) glGetString(GL_RENDERER));
	LogF("Verision: %s", (char*) glGetString(GL_VERSION));

#ifdef _WIN32
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	LogF("Number of processors: %d\n", SystemInfo.dwNumberOfProcessors);
#endif
}

void RenderingEngine::AddRendererForBlock(unsigned int BlockID, const char *BlockModelFileName)
{
	PlatformFileSystem::LoadModel(&m_CustomBlockRenderers[BlockID], BlockModelFileName);
}

void RenderingEngine::SetupGeometryPass()
{
	glGenFramebuffers(1, &m_GeometryFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryFBO);
    
	// We only need RGB in the frame buffer
	m_GeomPassPositionTexture.LoadFromBuffer((float*) NULL, m_ScreenWidth, m_ScreenHeight, GL_RGBA, GL_RGBA); // RGBA?
	// Nearest filtering since we want raw pixels not interpolations
	m_GeomPassPositionTexture.SetFilteringMode(GL_NEAREST);
	m_GeomPassPositionTexture.SetWrapMode(GL_CLAMP_TO_EDGE);
	m_GeomPassPositionTexture.m_BindingPoint = 0;
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GeomPassPositionTexture.m_TextureId, 0);

    
	// We only need RGB in the frame buffer
	m_GeomPassNormalTexture.LoadFromBuffer((float*)NULL, m_ScreenWidth, m_ScreenHeight, GL_RGB, GL_RGBA); // RGBA?
	// Nearest filtering since we want raw pixels not interpolations
	m_GeomPassNormalTexture.SetFilteringMode(GL_NEAREST);
	m_GeomPassNormalTexture.SetWrapMode(GL_CLAMP_TO_EDGE);
	m_GeomPassNormalTexture.m_BindingPoint = 1;
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GeomPassNormalTexture.m_TextureId, 0);
    
	// We only need RGB in the frame buffer
	m_GeomPassTexCoordTexture.LoadFromBuffer((float*)NULL, m_ScreenWidth, m_ScreenHeight, GL_RGB, GL_RGBA); // RGBA?
	// Nearest filtering since we want raw pixels not interpolations
	m_GeomPassTexCoordTexture.SetFilteringMode(GL_NEAREST);
	m_GeomPassTexCoordTexture.SetWrapMode(GL_CLAMP_TO_EDGE);
	m_GeomPassTexCoordTexture.m_BindingPoint = 2;
	// This binds the texture of m_GeometryGBufferTextures[i] to a color attachment in the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GeomPassTexCoordTexture.m_TextureId, 0);
    
	// Do the same as above for the depth texture
	m_GeomPassDepthTexture.LoadFromBuffer((float*)NULL, m_ScreenWidth, m_ScreenHeight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);
	m_GeomPassDepthTexture.SetFilteringMode(GL_NEAREST);
	m_GeomPassDepthTexture.SetWrapMode(GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GeomPassDepthTexture.m_TextureId, 0);
    
	// Specify which buffers we want to draw to
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(ArrayCount(DrawBuffers), DrawBuffers);
    
	// Make sure no one else modifies this frame buffer (kinda like VAOs)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderingEngine::SetupSSAOPass()
{
	glGenFramebuffers(1, &m_SSAOFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

	m_SSAOColorBuffer.LoadFromBuffer((float*)NULL, m_ScreenWidth, m_ScreenHeight, GL_RED, GL_RED);
	// Nearest filtering since we want raw pixels not interpolations
	m_SSAOColorBuffer.SetFilteringMode(GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOColorBuffer.m_TextureId, 0);

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

	m_SSAONoiseTexture.LoadFromBuffer(SSAONoise, 4, 4, GL_RGB32F, GL_RGB);
	m_SSAONoiseTexture.SetFilteringMode(GL_NEAREST);
	m_SSAONoiseTexture.SetWrapMode(GL_REPEAT);
	m_SSAONoiseTexture.m_BindingPoint = 2;
	
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

	glGenFramebuffers(1, &m_SSAOBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOBlurFBO);

	m_SSAOBlurColorBuffer.LoadFromBuffer((float*)NULL, m_ScreenWidth, m_ScreenHeight, GL_RED, GL_RGB); // GL_RED?
	m_SSAOBlurColorBuffer.SetFilteringMode(GL_NEAREST);
	m_SSAOBlurColorBuffer.m_BindingPoint = 3;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SSAOBlurColorBuffer.m_TextureId, 0);

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	glBindVertexArray(0);
}

void RenderingEngine::RenderFrame(World* RenderWorld, const float& DeltaTime)
{
	// Setup for the geometry pass
	StartGeometryPass();

	if (m_bWireframeRender)
	{
		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

    // Do the geometry pass (render everything)
	m_pChunkRenderer->RenderAllChunks(RenderWorld->m_pCurrentPlayer);
    
	if (m_bWireframeRender)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

    // Do the SSAO
    SSAOPass();

	// Do the light pass
	LightPass();
}

void RenderingEngine::SSAOPass()
{
	// Do the SSAO Pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	m_pSSAOShader->Bind();
	m_pSSAOShader->SetDefaultSamplers();
	m_pSSAOShader->SetProjectionMatrix(Camera::g_ActiveCamera->m_ProjectionMatrix);
	m_pSSAOShader->SetScreenDimension(glm::vec2(m_ScreenWidth, m_ScreenHeight));

	// Bind the textures from the gemoetry pass
	m_GeomPassPositionTexture.Use();
	m_GeomPassNormalTexture.Use();
	m_SSAONoiseTexture.Use(),
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
	m_SSAOColorBuffer.Use();

	// Render a screen quad
	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FORCEINLINE void RenderingEngine::StartGeometryPass()
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
	m_GeomPassPositionTexture.Use();

	// Bind the normal texture
	m_GeomPassNormalTexture.Use();

	// Bind the texture coord texture
	m_GeomPassTexCoordTexture.Use();

	m_SSAOBlurColorBuffer.Use();
	
	// Bind the texture atlas for the block textures
	m_TextureAtlas.Use();

	// Render a screen quad
	glBindVertexArray(m_ScreenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);
}

void RenderingEngine::UpdateScreenDimensions(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	m_ScreenWidth = NewWidth;
	m_ScreenHeight = NewHeight;
	glViewport(0, 0, NewWidth, NewHeight);

	DeleteFrameBuffers();

	SetupGeometryPass();

	SetupSSAOPass();
}

void RenderingEngine::DeleteFrameBuffers()
{
	// Delete the frame buffers
	glDeleteFramebuffers(1, &m_GeometryFBO);
	glDeleteFramebuffers(1, &m_SSAOFBO);
	glDeleteFramebuffers(1, &m_SSAOBlurFBO);
}
