#include <stdio.h>
#include <bitset>

#include "Platform/Platform.h"
#include "Game/World.h"
#include "Rendering\TextRender.h"
#include "Rendering\RenderingEngine.h"

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; }

World* g_World = NULL;
RenderingEngine* g_RenderingEngine = NULL;

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
	WindowParams.UseVSync = false;
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
	WindowParams.UseVSync = false;
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

	// Loads the font library
	std::string* Directory = PlatformFileSystem::GetAssetDirectory(DT_FONTS);
	LoadFontLibrary(Directory);

	// Loads the world and initializes subobjects
	g_World = new World();
	g_World->Initialize();

	g_RenderingEngine = new RenderingEngine();
	g_RenderingEngine->Initialize(WindowParams.UseDepthTest);

	TextRenderData2D* FPSCounter = TextRender::AddTextToRender("FPS: 0", 16.0f);
	
	double LastFrameTime = SDL_GetTicks() / 1000.0;
	double DeltaTime = 0.0;
	double CumulativeFrameTime = 0.0;
	uint16_t FramesPerSecond = 0;
	while (Window.PrepareForRender())
	{
		// Update the world with the last frames delta time
		g_World->Update(DeltaTime);

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
			TextRender::RemoveText(FPSCounter);
			char Buffer[48];
			sprintf(Buffer, "FPS: %d", FramesPerSecond);
			TextRender::AddTextToRender(Buffer, 16.0f);
			CumulativeFrameTime = 0;
			FramesPerSecond = 0;
		}
	}

	SAFE_DELETE(g_RenderingEngine);
	SAFE_DELETE(g_World);

	Window.DestroyWindow();
}