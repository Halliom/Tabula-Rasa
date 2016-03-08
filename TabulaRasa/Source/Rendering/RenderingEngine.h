#pragma once

#include "GL\glew.h"

#include "../Game/World.h"

enum GBUFFER_TEXTURE_LAYER
{
	GBUFFER_LAYER_POSITION,
	GBUFFER_LAYER_TEXCOORD,
	GBUFFER_LAYER_NUM
};

class RenderingEngine
{
public:

	~RenderingEngine();

	void Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight);

	void RenderFrame(World* RenderWorld, const float& DeltaTime);

	void StartRenderingIntoFrameBuffer();

	void StopRenderingIntoFrameBuffer();

	void ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight);

	unsigned int m_ScreenWidth;
	unsigned int m_ScreenHeight;

	GLuint m_FBO;
	GLuint m_GBufferTextures[GBUFFER_LAYER_NUM];
	GLuint m_DepthTexture;

};