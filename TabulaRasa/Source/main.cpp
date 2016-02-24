#include <chrono>
#include <stdio.h>
#include <bitset>

#include "Platform/Platform.h"
#include "Game/World.h"

#define println(str) OutputDebugStringA(str)

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
	PlatformWindow Window = PlatformWindow({ "Testing", 800, 600, false, false, hInstance });
#else
int main(int argc, char* argv[])
{
	PlatformWindow Window = PlatformWindow({ "Testing", 640, 480, true, false });
#endif
	bool Success = Window.SetupWindowAndRenderContext();
	if (!Success)
	{
		//TODO: Do something with this
		Window.GetErrorMessage();
	}

	World WorldObject;

	float LastFrameTime = ((float) GetTickCount()) / 1000.0f;
	float DeltaTime = 0.0f;
	float CumulativeFrameTime = 0.0f;
	uint16_t FramesPerSecond = 0;
	while (Window.PrepareForRender())
	{
		// Update the world with the last frames delta time
		WorldObject.Update(DeltaTime);

		// Swap the buffers
		Window.PostRender();

		float CurrentTime = ((float)GetTickCount()) / 1000.0f;
		DeltaTime = CurrentTime - LastFrameTime;
		LastFrameTime = ((float) GetTickCount()) / 1000.0f;
		
		CumulativeFrameTime += DeltaTime;
		++FramesPerSecond;
		if (CumulativeFrameTime >= 1.0f)
		{
			CumulativeFrameTime = 0;

			char OutputString[256];
			sprintf_s(OutputString, "FPS: %d\n", FramesPerSecond);
			OutputDebugStringA(OutputString);
			FramesPerSecond = 0;
		}
	}
}