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
#include "Rendering/RenderingEngine.h"

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; }

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
	WindowParams.Instance = hInstance;
#else
int main(int argc, char* argv[])
{
	WindowParameters WindowParams;
	WindowParams.Title = "Tabula Rasa";
	WindowParams.Width = 960;
	WindowParams.Height = 540;
	WindowParams.UseVSync = true;
	WindowParams.UseDepthTest = false;
	WindowParams.Fullscreen = false;
	WindowParams.StartMaximized = false;
#endif
    g_Engine = new EngineGlobals();
    
	g_Engine->g_MemoryManager = new GameMemoryManager();
	if (!g_Engine->g_MemoryManager->Initialize())
	{
		// Failure to load memory
		assert(false);
	}

	// TODO: Create Settings class
	Script Preferences = Script("preferences.lua");
	WindowParams.Title = Preferences.GetStringFromTable("Window", "title").c_str();
	WindowParams.Width = Preferences.GetIntFromTable("Window", "width") / 2;
	WindowParams.Height = Preferences.GetIntFromTable("Window", "height") / 2;
	WindowParams.UseVSync = Preferences.GetBoolFromTable("Window", "vsync");
	//WindowParams.Fullscreen = Preferences.GetBoolFromTable("Window", "full_screen");
	//WindowParams.StartMaximized = Preferences.GetBoolFromTable("Window", "start_maximized");

	PlatformWindow Window = PlatformWindow(WindowParams);
	bool Success = Window.SetupWindowAndRenderContext();
	if (!Success)
	{
		//TODO: Do something with this
		Window.GetErrorMessage();
	}

	// Loads the font library
	std::string Directory = PlatformFileSystem::GetAssetDirectory(DT_FONTS);
	FontLibrary::g_FontLibrary = new FontLibrary();
	FontLibrary::g_FontLibrary->Initialize(Directory);
	FontLibrary::g_FontLibrary->LoadFontFromFile("TitilliumWeb-Regular.ttf", 20);
	FontLibrary::g_FontLibrary->LoadFontFromFile("RobotoMono-Regular.ttf", 18);

	g_Engine->g_Console = new Console();

	g_Engine->g_RenderingEngine = new RenderingEngine();
	g_Engine->g_RenderingEngine->Initialize(Window.WindowParams.Width, Window.WindowParams.Height);
	g_Engine->g_RenderingEngine->AddRendererForBlock(3, "Chest_Model.obj");

	g_Engine->g_GUIRenderer = new DebugGUIRenderer((int)Window.WindowParams.Width, (int)Window.WindowParams.Height);

	// Loads the world and initializes subobjects
	g_Engine->g_World = new World();
	g_Engine->g_World->Initialize();

	g_Engine->g_RenderingEngine->PostInitialize();

	double LastFrameTime = SDL_GetTicks() / 1000.0;
	double DeltaTime = 0.0;
	double CumulativeFrameTime = 0.0;
	uint16_t FramesPerSecond = 0;
	int StaticFPS = 0.0f;
	while (Window.PrepareForRender())
	{
		g_Engine->g_GUIRenderer->BeginFrame();

		// Update the world with the last frames delta time
		g_Engine->g_World->Update(DeltaTime);

		// Render the world
		g_Engine->g_RenderingEngine->RenderFrame(g_Engine->g_World, DeltaTime);
        
        // Render all GUI elements
		g_Engine->g_GUIRenderer->RenderFrame(StaticFPS, (float)DeltaTime);

		// Swap the buffers
		Window.PostRender();

		double CurrentTime = SDL_GetTicks() / 1000.0;
		DeltaTime = CurrentTime - LastFrameTime;
		LastFrameTime = SDL_GetTicks() / 1000.0;

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
	FontLibrary::g_FontLibrary->Destroy();

	SAFE_DELETE(g_Engine->g_World);
    SAFE_DELETE(g_Engine->g_GUIRenderer);
	SAFE_DELETE(g_Engine->g_RenderingEngine);
	SAFE_DELETE(g_Engine->g_Console);
	SAFE_DELETE(g_Engine->g_MemoryManager);
    SAFE_DELETE(g_Engine);

	Window.DestroyWindow();
}
