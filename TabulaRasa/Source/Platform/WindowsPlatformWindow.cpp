#include "Platform.h"

#include "glm\common.hpp"

#include "../Engine/Input.h"
#include "../Engine/Camera.h"

#include "../Rendering/Fonts.h"

#include "../Rendering/RenderingEngine.h"

PlatformWindow* PlatformWindow::GlobalWindow = NULL;

extern RenderingEngine* g_RenderingEngine;

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
	
	uint32_t Flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_ALLOW_HIGHDPI;

	if (WindowParams.Fullscreen)
	{
		Flags |= SDL_WINDOW_FULLSCREEN;
	}
	if (WindowParams.StartMaximized)
	{
		Flags |= SDL_WINDOW_MAXIMIZED;
	}

	// Set the OpenGL version to 330 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	MainWindow = SDL_CreateWindow(WindowParams.Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowParams.Width, WindowParams.Height, Flags);
	if (MainWindow == NULL)
	{
		//TODO: Print to log since rendering isn't initialized yet
		return false;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	// Initializes the input for the mouse
	int MouseX = 0, MouseY = 0;
	SDL_GetMouseState(&MouseX, &MouseY);
	Input::MouseX = MouseX;
	Input::MouseY = MouseY;

	// Create the OpenGL context to draw to
	MainContext = SDL_GL_CreateContext(MainWindow);

	SDL_GL_SetSwapInterval(1);

	glewExperimental = true;
	glewInit();

	if (GlobalWindow->WindowParams.UseVSync)
	{
		SDL_GL_SetSwapInterval(1);
	}
	else
	{
		SDL_GL_SetSwapInterval(0);
	}

	return true;
}

void PlatformWindow::DestroyWindow()
{
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
			case SDL_WINDOWEVENT:
			{
				switch (Event.window.event)
				{
					case SDL_WINDOWEVENT_SIZE_CHANGED:
					case SDL_WINDOWEVENT_RESIZED:
					{
						glViewport(0, 0, Event.window.data1, Event.window.data2);

						if (Camera::ActiveCamera)
						{
							Camera::ActiveCamera->WindowWidth = Event.window.data1;
							Camera::ActiveCamera->WindowHeight = Event.window.data2;
							Camera::ActiveCamera->IsScreenMatrixDirty = true;
							Camera::ActiveCamera->IsProjectionMatrixDirty = true;

							g_RenderingEngine->ScreenDimensionsChanged(Event.window.data1, Event.window.data2);
						}

						WindowParams.Width = Event.window.data1;
						WindowParams.Height = Event.window.data2;
						break;
					}
				}
				break;
			}
			case SDL_KEYDOWN:
			{
#ifdef _DEBUG
				if (Event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					return false;
				}
#endif
				SDL_SetRelativeMouseMode(SDL_TRUE);

				Input::Keys[Event.key.keysym.scancode] = true;
				break;
			}
			case SDL_KEYUP:
			{
				Input::Keys[Event.key.keysym.scancode] = false;
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				Input::MouseButtons[Event.button.button] = true;
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				Input::MouseButtons[Event.button.button] = false;
				break;
			}
			case SDL_MOUSEMOTION:
			{
				Input::MouseX += Event.motion.xrel;
				Input::MouseY += Event.motion.yrel;
				break;
			}
		}
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
