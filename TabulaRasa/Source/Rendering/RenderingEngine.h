#pragma once

#include "GL\glew.h"

#include "../Game/World.h"
#include "../Platform/Platform.h"
#include "../Engine/Block.h"

enum GBufferTextureLayerGeometryPass
{
	GBUFFER_LAYER_GEOMETRY_POSITION,
	GBUFFER_LAYER_GEOMETRY_NORMAL,
	GBUFFER_LAYER_GEOMETRY_TEXCOORD,
	GBUFFER_LAYER_GEOMETRY_NUM
};

class RenderingEngine
{
public:

	~RenderingEngine();

	void Initialize(const unsigned int &ScreenWidth, const unsigned int &ScreenHeight);

	void AddRendererForBlock(unsigned int BlockID, const char *BlockModelFileName);

	void SetupGeometryPass();

	void SetupSSAOPass();

	void SetupQuad();

	void RenderFrame(World* RenderWorld, const float& DeltaTime);

	void StartGeometryPass();

	void SSAOPass();

	void LightPass();

	void ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight);

	unsigned int m_ScreenWidth;
	unsigned int m_ScreenHeight;

	GLuint m_GeometryFBO;
	GLuint m_GeometryGBufferTextures[GBUFFER_LAYER_GEOMETRY_NUM];
	GLuint m_GeometryDepthTexture;

	GLShaderProgram* m_pLightPassShader;

	GLuint m_SSAOFBO;
	GLuint m_SSAONoiseTexture;
	GLuint m_SSAOColorBuffer;

	GLShaderProgram* m_pSSAOShader;

	float m_pSSAOKernel[192];

	GLuint m_SSAOBlurFBO;
	GLuint m_SSAOBlurColorBuffer;

	GLuint m_ScreenQuadVBO;
	GLuint m_ScreenQuadIBO;
	GLuint m_ScreenQuadVAO;

	GLShaderProgram* m_pSSAOBlurShader;

	LoadedModel CustomBlockRenderers[MAX_NUM_BLOCKS];

};
