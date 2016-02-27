#pragma once

#include "windows.h" // Need to include windows.h before gl.h
#include "windowsx.h"
#include "GL\glew.h"
#include "GL\wglew.h"

struct WindowParameters
{
	const char Title[64];
	unsigned int Width;
	unsigned int Height;
	bool UseVSync;
	bool UseDepthTest;
	HINSTANCE Instance;
};

class PlatformWindow
{
public:
	PlatformWindow(const WindowParameters& WindowParams);

	~PlatformWindow();

	bool SetupWindowAndRenderContext();

	void GetErrorMessage();

	bool PrepareForRender();

	void PostRender();

	inline static LRESULT CALLBACK WindowProcessMessages(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam);

	WindowParameters WindowParams;

private:

	static PlatformWindow* GlobalWindow;

	HGLRC GLRenderContext;

	HDC DeviceContext;
};