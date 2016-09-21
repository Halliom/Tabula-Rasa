#pragma once

#include "GL/glew.h"

#include "../Platform/Platform.h"
#include "../Engine/Async/ThreadSystem.h"
#include "../Engine/Async/Mutex.h"
#include "../Engine/Block.h"
#include "../Rendering/Texture.h"

class ChunkRenderer;
class SSAOShader;
class SSAOBlurShader;
class LightPassShader;

class RenderingEngine
{
public:
    
	~RenderingEngine();

	void Initialize(const unsigned int &ScreenWidth, const unsigned int &ScreenHeight);

	void PostInitialize();

	void AddRendererForBlock(unsigned int BlockID, const char *BlockModelFileName);
    
    void ScheduleRenderJob(IJob* RenderJob);

	void SetupGeometryPass();

	void SetupSSAOPass();

    void SetupQuad();

	void StartGeometryPass();

	void SSAOPass();

	void LightPass();
    
    void RenderFrame(class World* RenderWorld, const float& DeltaTime);

	void UpdateScreenDimensions(const unsigned int& NewWidth, const unsigned int& NewHeight);
    
    ChunkRenderer*      m_pChunkRenderer;
    LoadedModel         m_CustomBlockRenderers[MAX_NUM_BLOCKS];
    
private:

	void DeleteFrameBuffers();

	unsigned int        m_ScreenWidth;
	unsigned int        m_ScreenHeight;
	bool				m_bWireframeRender;
    
    JobQueue            m_RenderJobs;
    TicketMutex         m_QueueMutex;

	GLuint              m_GeometryFBO;
	Texture				m_GeomPassPositionTexture;
	Texture				m_GeomPassNormalTexture;
	Texture				m_GeomPassTexCoordTexture;
	Texture             m_GeomPassDepthTexture;

    Texture             m_TextureAtlas;

	LightPassShader*    m_pLightPassShader;

	GLuint              m_SSAOFBO;
	Texture             m_SSAONoiseTexture;
	Texture             m_SSAOColorBuffer;
	SSAOShader*         m_pSSAOShader;

	float               m_pSSAOKernel[192];
	GLuint              m_SSAOBlurFBO;
	Texture             m_SSAOBlurColorBuffer;
    SSAOBlurShader*     m_pSSAOBlurShader;

	GLuint              m_ScreenQuadVBO;
	GLuint              m_ScreenQuadIBO;
	GLuint              m_ScreenQuadVAO;
};
