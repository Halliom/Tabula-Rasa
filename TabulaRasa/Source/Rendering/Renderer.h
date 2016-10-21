#pragma once

#include "GL/glew.h"
#include "glm\common.hpp"

#include "../Platform/Platform.h"
#include "../Engine/Async/ThreadSystem.h"
#include "../Engine/Async/Mutex.h"
#include "../Engine/Block.h"
#include "../Rendering/Texture.h"

class ChunkRenderer;

class Renderer
{
public:

	Renderer();

	~Renderer();

	void Initialize(const unsigned int &ScreenWidth, const unsigned int &ScreenHeight);

	void PostInitialize();

	/* Adds a special renderer for a certain Block */
	void AddCustomRendererForBlock(IDType BlockID, const char* BlockModelFilename);

	/* Schedules a Job to be executed on the rendering thread */
	void ScheduleRenderJob(IJob* RenderJob);

	/* Renders a frame */
	void RenderFrame(class World* RenderWorld, const float& DeltaTime);
	
	/* Called when the screen dimensions change (resized or resolution changed) */
	void ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight);
	void ScreenDimensionsChanged(const glm::ivec2 NewDimensions);

	/* The global chunk renderer instance */
	ChunkRenderer*      m_pChunkRenderer;

private:

	/* The current screen dimensions to render to */
	glm::ivec2			m_ScreenDimensions;

	/* Whether to render in debug wireframe mode or not */
	bool				m_bWireframeRender;

	/* Jobs to add to be executed by the rendering thread */
	JobQueue            m_RenderJobs;
	TicketMutex         m_QueueMutex;
};
