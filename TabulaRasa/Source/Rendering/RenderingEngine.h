#pragma once

#include "GL/glew.h"

#include "../Platform/Platform.h"
#include "../Engine/Block.h"
#include "../Rendering/Shader.h"
#include "../Rendering/Texture.h"

class ChunkRenderer;

class RenderingEngine
{
public:
    
	~RenderingEngine();

	void Initialize(const unsigned int &ScreenWidth, const unsigned int &ScreenHeight);

	void PostInitialize();

	void AddRendererForBlock(unsigned int BlockID, const char *BlockModelFileName);

	void SetupGeometryPass();

	void SetupSSAOPass();

	void SetupQuad();

	void RenderFrame(class World* RenderWorld, const float& DeltaTime);

	void StartGeometryPass();

	void SSAOPass();

	void LightPass();

	void UpdateScreenDimensions(const unsigned int& NewWidth, const unsigned int& NewHeight);
    
    ChunkRenderer*      m_pChunkRenderer;
    LoadedModel         m_CustomBlockRenderers[MAX_NUM_BLOCKS];
    
private:

	void DeleteFrameBuffers();

	unsigned int        m_ScreenWidth;
	unsigned int        m_ScreenHeight;
	bool				m_bWireframeRender;

	GLuint              m_GeometryFBO;
	Texture				m_GeomPassPositionTexture;
	Texture				m_GeomPassNormalTexture;
	Texture				m_GeomPassTexCoordTexture;
	Texture             m_GeomPassDepthTexture;

    Texture             m_TextureAtlas;

	GLShaderProgram*    m_pLightPassShader;

	GLuint              m_SSAOFBO;
	Texture             m_SSAONoiseTexture;
	Texture             m_SSAOColorBuffer;
	GLShaderProgram*    m_pSSAOShader;

	float               m_pSSAOKernel[192];
	GLuint              m_SSAOBlurFBO;
	Texture             m_SSAOBlurColorBuffer;
    GLShaderProgram*    m_pSSAOBlurShader;

	GLuint              m_ScreenQuadVBO;
	GLuint              m_ScreenQuadIBO;
	GLuint              m_ScreenQuadVAO;
};
