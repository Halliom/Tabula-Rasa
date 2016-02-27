#include "Platform.h"

#include "glm\common.hpp"

#include "../Engine/Input.h"

#include "../Rendering/Fonts.h"

PlatformWindow* PlatformWindow::GlobalWindow = NULL;

PlatformWindow::PlatformWindow(const WindowParameters& WindowParams) : 
	WindowParams(WindowParams)
{
	GlobalWindow = this;
}

PlatformWindow::~PlatformWindow()
{
	
}

bool PlatformWindow::SetupWindowAndRenderContext()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		//TODO: Print to log since rendering isn't initialized yet
		return false;
	}
	
	uint32_t Flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

	if (WindowParams.Fullscreen)
	{
		Flags |= SDL_WINDOW_FULLSCREEN;
	}
	if (WindowParams.StartMaximized)
	{
		Flags |= SDL_WINDOW_MAXIMIZED;
	}

	MainWindow = SDL_CreateWindow(WindowParams.Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowParams.Width, WindowParams.Height, Flags);
	if (MainWindow == NULL)
	{
		//TODO: Print to log since rendering isn't initialized yet
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Create the OpenGL context to draw to
	MainContext = SDL_GL_CreateContext(MainWindow);

	SDL_GL_SetSwapInterval(1);

	//TODO: Move into rendering engine

	glewInit();

	if (GlobalWindow->WindowParams.UseVSync)
	{
		SDL_GL_SetSwapInterval(1);
	}
	else
	{
		SDL_GL_SetSwapInterval(0);
	}

	std::string* Directory = PlatformFileSystem::GetAssetDirectory(DT_FONTS);

	LoadFontLibrary(Directory);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	if (GlobalWindow->WindowParams.UseDepthTest)
	{
		glEnable(GL_DEPTH_TEST);
	}

	return true;
}

void PlatformWindow::DestroyWindow()
{
	UnloadFontLibrary();

	SDL_GL_DeleteContext(MainContext);

	SDL_DestroyWindow(MainWindow);

	SDL_Quit();
}

void PlatformWindow::GetErrorMessage()
{
	
}

bool PlatformWindow::PrepareForRender()
{	
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_QUIT)
			return false;

		switch (Event.type)
		{
			case SDL_KEYDOWN:
			{
				break;
			}
			case SDL_KEYUP:
			{
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				break;
			}
			case SDL_MOUSEMOTION:
			{
				break;
			}
		}
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

void PlatformWindow::PostRender()
{
	SDL_GL_SwapWindow(MainWindow);
}

LRESULT PlatformWindow::WindowProcessMessages(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_SIZE:
		{
			// Height is in the high-order of lParam, Width is in the low-order
			unsigned int NewWidth = LOWORD(lParam);
			unsigned int NewHeight = HIWORD(lParam);
			glViewport(0, 0, NewWidth, NewHeight);
			break;
		}
		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				// Screensaver Trying To Start?
				case SC_SCREENSAVE:
				// Monitor Trying To Enter Powersave?
				case SC_MONITORPOWER:
				return 0;	// Prevent From Happening
			}
			break;
		}
	}
	return 0;
}
