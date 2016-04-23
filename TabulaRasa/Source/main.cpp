#include <stdio.h>
#include <bitset>

#include "Platform/Platform.h"
#include "Game/World.h"
#include "Engine\Console.h"
#include "Rendering\TextRenderer.h"
#include "Rendering\RenderingEngine.h"
#include "Engine\ScriptEngine.h"
#include "Engine\Core\Memory.h"

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; }

World* g_World = NULL;
RenderingEngine* g_RenderingEngine = NULL;
Console* g_Console = NULL;
GameMemoryManager* g_MemoryManager = NULL;

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
	WindowParameters WindowParams;
	WindowParams.Title = "Tabula Rasa";
	WindowParams.Width = 1280;
	WindowParams.Height = 720;
	WindowParams.UseVSync = true;
	WindowParams.UseDepthTest = false;
	WindowParams.Fullscreen = false;
	WindowParams.StartMaximized = false;
	WindowParams.Instance = hInstance;
#else
int main(int argc, char* argv[])
{
	WindowParameters WindowParams;
	WindowParams.Title = "Tabula Rasa";
	WindowParams.Width = 1280;
	WindowParams.Height = 720;
	WindowParams.UseVSync = true;
	WindowParams.UseDepthTest = false;
	WindowParams.Fullscreen = false;
	WindowParams.StartMaximized = false;
#endif
	PlatformWindow Window = PlatformWindow(WindowParams);
	bool Success = Window.SetupWindowAndRenderContext();
	if (!Success)
	{
		//TODO: Do something with this
		Window.GetErrorMessage();
	}

	g_MemoryManager = new GameMemoryManager();
	if (!g_MemoryManager->Initialize())
	{
		assert(false, "Failure to load memory");
	}

	// Loads the font library
	std::string Directory = PlatformFileSystem::GetAssetDirectory(DT_FONTS);
	FontLibrary::g_FontLibrary = new FontLibrary();
	FontLibrary::g_FontLibrary->Initialize(Directory);
	FontLibrary::g_FontLibrary->LoadFontFromFile("TitilliumWeb-Regular.ttf", 20);
	FontLibrary::g_FontLibrary->LoadFontFromFile("RobotoMono-Regular.ttf", 18);

	g_Console = new Console();
	g_Console->OnUpdateInputMode();

	g_RenderingEngine = new RenderingEngine();
	g_RenderingEngine->Initialize(WindowParams.Width, WindowParams.Height);
	g_RenderingEngine->AddRendererForBlock(3, "Chest_Model.obj");

	// Loads the world and initializes subobjects
	g_World = new World();
	g_World->Initialize();

	g_RenderingEngine->PostInitialize();

	TextRenderData2D* FPSCounter = TextRenderer::AddTextToRenderWithColor("FPS: 0", 8.0f, 8.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

	double LastFrameTime = SDL_GetTicks() / 1000.0;
	double DeltaTime = 0.0;
	double CumulativeFrameTime = 0.0;
	uint16_t FramesPerSecond = 0;
	while (Window.PrepareForRender())
	{
		// Update the world with the last frames delta time
		g_World->Update(DeltaTime);

		// Render the world
		g_RenderingEngine->RenderFrame(g_World, DeltaTime);

		// Swap the buffers
		Window.PostRender();

		double CurrentTime = SDL_GetTicks() / 1000.0;
		DeltaTime = CurrentTime - LastFrameTime;
		LastFrameTime = SDL_GetTicks() / 1000.0;

		CumulativeFrameTime = CumulativeFrameTime + DeltaTime;
		++FramesPerSecond;
		if (CumulativeFrameTime >= 1.0f)
		{
			TextRenderer::RemoveText(FPSCounter);
			char Buffer[48];
			sprintf(Buffer, "FPS: %d", FramesPerSecond);
			FPSCounter = TextRenderer::AddTextToRenderWithColor(Buffer, 8.0f, 8.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
			CumulativeFrameTime = 0;
			FramesPerSecond = 0;
		}
		g_MemoryManager->ClearTransientMemory();
	}
	FontLibrary::g_FontLibrary->Destroy();

	SAFE_DELETE(g_World);
	SAFE_DELETE(g_RenderingEngine);
	SAFE_DELETE(g_Console);
	SAFE_DELETE(g_MemoryManager);

	Window.DestroyWindow();
}
