#pragma once

#include "windows.h" // Need to include windows.h before gl.h
#include "windowsx.h"

#include "SDL2\SDL.h"

#include "GL\glew.h"

struct WindowParameters
{
	const char* Title;
	unsigned int Width;
	unsigned int Height;
	bool UseVSync;
	bool UseDepthTest;
	bool Fullscreen;
	bool StartMaximized;
	HINSTANCE Instance;
};

class PlatformWindow
{
public:
	PlatformWindow(const WindowParameters& WindowParams);

	~PlatformWindow();

	bool SetupWindowAndRenderContext();

	void DestroyWindow();

	void GetErrorMessage();

	bool PrepareForRender();

	void PostRender();

	SDL_Window* GetWindow();

	inline static LRESULT CALLBACK WindowProcessMessages(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam);

	WindowParameters WindowParams;

	static PlatformWindow* GlobalWindow;

private:

	SDL_Window* MainWindow;

	SDL_GLContext MainContext;
};