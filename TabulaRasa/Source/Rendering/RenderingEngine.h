#pragma once

#include "GL\glew.h"

#include "../Game/World.h"

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

	void Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight);

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

	GLuint m_SSAOFBO;
	GLuint m_SSAONoiseTexture;
	GLuint m_SSAOColorBuffer;

	float m_pSSAOKernel[192];

	GLuint m_ScreenQuadVBO;
	GLuint m_ScreenQuadIBO;
	GLuint m_ScreenQuadVAO;

	GLShaderProgram* m_pLightPassShader;

	GLShaderProgram* m_pSSAOShader;

};
