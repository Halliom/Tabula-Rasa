#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1

#include <stdio.h>
#include <bitset>

#include "Platform/Platform.h"

#include "Game/World.h"

#include "Engine/Engine.h"
#include "Engine/Console.h"
#include "Engine/ScriptEngine.h"
#include "Engine/Core/Memory.h"
#include "Engine/Core/Random.h"

#include "Rendering/GuiSystem.h"
#include "Rendering/Renderer.h"
#include "Engine/Async/Thread.h"

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; }

#define NUM_WORKER_THREADS 7

/*
 * All of the global singletons reside inside this class which
 * gets initialized here in the main functon
 */
EngineGlobals* g_Engine = NULL;

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
	WindowParams.StartMaximized = true;
#endif
    g_Engine = new EngineGlobals();
    
	g_Engine->g_MemoryManager = new GameMemoryManager();
	if (!g_Engine->g_MemoryManager->Initialize())
	{
		// Failure to load memory
		assert(false);
	}
    
    g_Engine->g_ScriptEngine = new ScriptEngine();
    g_Engine->g_ScriptEngine->ExecuteScript("preferences.lua");
    LuaTable* WindowTable = g_Engine->g_ScriptEngine->GetTable("Window");
	// TODO: Create Settings class
	WindowParams.Title = WindowTable->GetValue("title").String;
	WindowParams.Width = (int)WindowTable->GetValue("width").Number;
	WindowParams.Height = (int)WindowTable->GetValue("height").Number;
	WindowParams.UseVSync = WindowTable->GetValue("vsync").Boolean;
	WindowParams.Fullscreen = WindowTable->GetValue("full_screen").Boolean;
	WindowParams.StartMaximized = WindowTable->GetValue("start_maximized").Boolean;
#ifdef __APPLE__
	WindowParams.Width /= 2;
	WindowParams.Height /= 2;
#endif

	PlatformWindow Window = PlatformWindow(WindowParams);
	bool Success = Window.SetupWindowAndRenderContext();
	if (!Success)
	{
		//TODO: Do something with this
		Window.GetErrorMessage();
	}
    
    // Loads the font library
    std::string Directory = PlatformFileSystem::GetAssetDirectory(DT_FONTS);
    g_Engine->g_FontLibrary = new FontLibrary();
    g_Engine->g_FontLibrary->Initialize(Directory);
    g_Engine->g_FontLibrary->LoadFontFromFile("TitilliumWeb-Regular.ttf", 20);
    g_Engine->g_FontLibrary->LoadFontFromFile("RobotoMono-Regular.ttf", 18);

    // Creates the console
	g_Engine->g_Console = new Console();

    // Initializes the rendering engine
	g_Engine->g_WorldRenderer = new Renderer();
	g_Engine->g_WorldRenderer->Initialize(Window.WindowParams.Width, Window.WindowParams.Height);
	g_Engine->g_WorldRenderer->AddCustomRendererForBlock(3, "Chest_Model.obj");

	g_Engine->g_GUIRenderer = new DebugGUIRenderer((int)Window.WindowParams.Width, (int)Window.WindowParams.Height);

	// Loads the world and initializes subobjects
	g_Engine->g_World = new World();
	g_Engine->g_World->Initialize();

	g_Engine->g_WorldRenderer->PostInitialize();

	double LastFrameTime = glfwGetTime();
	double DeltaTime = 0.0;
	double CumulativeFrameTime = 0.0;
	uint16_t FramesPerSecond = 0;
	int StaticFPS = 0;
	
	ThreadSystem::InitializeThreads(NUM_WORKER_THREADS);

	while (Window.PrepareForRender())
	{
		g_Engine->g_GUIRenderer->BeginFrame(DeltaTime);

		// Update the world with the last frames delta time
		g_Engine->g_World->Update(DeltaTime);

		// Render the world
		g_Engine->g_WorldRenderer->RenderFrame(g_Engine->g_World, DeltaTime);
        
        // Render all GUI elements
		g_Engine->g_GUIRenderer->RenderFrame(StaticFPS, (float)DeltaTime);

		// Swap the buffers
		Window.PostRender();

		double CurrentTime = glfwGetTime();
		DeltaTime = CurrentTime - LastFrameTime;
		LastFrameTime = glfwGetTime();

		CumulativeFrameTime = CumulativeFrameTime + DeltaTime;
		++FramesPerSecond;
		if (CumulativeFrameTime >= 1.0f)
		{
			CumulativeFrameTime = 0;
			StaticFPS = FramesPerSecond;
			FramesPerSecond = 0;
		}
		g_Engine->g_MemoryManager->ClearTransientMemory();
	}

	ThreadSystem::DestroyThreads();

	SAFE_DELETE(g_Engine->g_World);
    SAFE_DELETE(g_Engine->g_GUIRenderer);
	SAFE_DELETE(g_Engine->g_WorldRenderer);
	SAFE_DELETE(g_Engine->g_Console);
    SAFE_DELETE(g_Engine->g_FontLibrary);
    SAFE_DELETE(g_Engine->g_ScriptEngine);
	SAFE_DELETE(g_Engine->g_MemoryManager);
    SAFE_DELETE(g_Engine);

	Window.DestroyWindow();
}
