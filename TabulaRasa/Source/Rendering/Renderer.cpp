#include "Renderer.h"

#include "../Game/World.h"
#include "../Game/Player.h"
#include "../Engine/Async/Job.h"
#include "../Engine/Console.h"
#include "../Engine/Camera.h"
#include "../Engine/Core/List.h"
#include "GuiSystem.h"
#include "ChunkRenderer.h"

#include "Shaders/SSAOShader.h"
#include "Shaders/SSAOBlurShader.h"
#include "Shaders/LightPassShader.h"

#include "GL/glew.h"

Renderer::Renderer() :
	m_ScreenDimensions(glm::ivec2(0, 0)),
	m_bWireframeRender(false)
{
	m_pChunkRenderer = new ChunkRenderer();
}

Renderer::~Renderer()
{
	delete m_pChunkRenderer;
}

void Renderer::Initialize(const unsigned int& ScreenWidth, const unsigned int& ScreenHeight)
{
	ScreenDimensionsChanged(ScreenWidth, ScreenHeight);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	// Nothingness is the "background"
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Initilize the chunk renderer
	m_pChunkRenderer->SetupChunkRenderer();
}

void Renderer::PostInitialize()
{
	GLint GLMajor = 0, GLMinor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &GLMajor);
	glGetIntegerv(GL_MINOR_VERSION, &GLMinor);
	Log("Rendering system initialized on system:");
	LogF("[GL] Vendor: %s", (char*)glGetString(GL_VENDOR));
	LogF("[GL] Renderer: %s", (char*)glGetString(GL_RENDERER));
	LogF("[GL] Version: %s", (char*)glGetString(GL_VERSION));
	LogF("[GL] Extensions: %s", (char*)glGetString(GL_EXTENSIONS));
	LogF("[GL] Rendercontext: (%d.%d)", GLMajor, GLMinor);
}

void Renderer::AddCustomRendererForBlock(IDType BlockID, const char* BlockModelFilename)
{
	m_pChunkRenderer->AddCustomRendererForBlock(BlockID, BlockModelFilename);
}

void Renderer::ScheduleRenderJob(IJob* RenderJob)
{
	SCOPED_LOCK(m_QueueMutex);

	m_RenderJobs.push_back(RenderJob);
}

void Renderer::RenderFrame(World* RenderWorld, const float& DeltaTime)
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enter mutex so access to m_RenderJobs is locked
	m_QueueMutex.Enter();

	while (!m_RenderJobs.empty())
	{
		// Get the next available render job
		IJob* NextRenderJob = m_RenderJobs.back();
		m_RenderJobs.pop_back();

		// Hand the mutex back
		m_QueueMutex.Leave();

		NextRenderJob->Execute();

		m_QueueMutex.Enter();
	}

	// Whenever we fall out we are always in a mutex so leave
	m_QueueMutex.Leave();

	// Render as wireframe if necessary
	if (m_bWireframeRender)
	{
		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Do the geometry pass (render everything)
	m_pChunkRenderer->RenderChunks(RenderWorld->m_pCurrentPlayer);

	// Un-toggle wireframe rendering for GUI to render correctly and so on
	if (m_bWireframeRender)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Renderer::ScreenDimensionsChanged(const unsigned int& NewWidth, const unsigned int& NewHeight)
{
	ScreenDimensionsChanged(glm::ivec2(NewWidth, NewHeight));
}

void Renderer::ScreenDimensionsChanged(const glm::ivec2 NewDimensions)
{
	m_ScreenDimensions = NewDimensions;

	// Update the viewport we are rendering to
	glViewport(0, 0, m_ScreenDimensions.x, m_ScreenDimensions.y);
}
