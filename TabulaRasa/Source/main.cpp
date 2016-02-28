#include <chrono>
#include <stdio.h>
#include <bitset>

#include "Platform/Platform.h"
#include "Game/World.h"
#include "Rendering\TextRender.h"

#define println(str) OutputDebugStringA(str)

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

double GetTimeMicroseconds()
{
	LARGE_INTEGER PerformanceCounter;
	LARGE_INTEGER PerformanceFreq;
	QueryPerformanceFrequency(&PerformanceFreq);
	QueryPerformanceCounter(&PerformanceCounter);

	PerformanceCounter.QuadPart *= 1000000; // Multiply with 10^6
	PerformanceCounter.QuadPart /= PerformanceFreq.QuadPart;
	return (double) PerformanceCounter.QuadPart / 1000000.0;
}

double GetTimeMilliseconds()
{
	auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;
}

int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
	PlatformWindow Window = PlatformWindow({ "Testing", 1280, 720, false, false, false, false, hInstance });
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
	TextRenderData2D* FPSCounter = TextRender::AddTextToRender("Hello World", 16.0f);

	//auto start = std::chrono::high_resolution_clock::now();
	//auto finish = std::chrono::high_resolution_clock::now();
	//std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n"

	clock_t t1 =	clock();
	
	double LastFrameTime = SDL_GetTicks() / 1000.0;
	double DeltaTime = 0.0;
	double CumulativeFrameTime = 0.0;
	uint16_t FramesPerSecond = 0;
	while (Window.PrepareForRender())
	{
		// Update the world with the last frames delta time
		WorldObject.Update(DeltaTime);

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
			TextRender::AddTextToRender(Buffer, 32.0f);
			CumulativeFrameTime = 0;
			FramesPerSecond = 0;
		}
	}
}