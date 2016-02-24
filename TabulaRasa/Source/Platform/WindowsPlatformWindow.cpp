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
	WNDCLASS WindowClass = { 0 };
	WindowClass.lpfnWndProc = PlatformWindow::WindowProcessMessages;
	WindowClass.hInstance = WindowParams.Instance;
	WindowClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	WindowClass.lpszClassName = "OctreeTestWindowClass";
	WindowClass.style = CS_OWNDC;

	if (!RegisterClass(&WindowClass))
	{
		return false;
	}

	HWND Window = CreateWindowA(
		WindowClass.lpszClassName,
		WindowParams.Title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0,
		0,
		WindowParams.Width,
		WindowParams.Height,
		0,
		0,
		WindowParams.Instance,
		0);
	
	if (Window)
		return true;
	else
		return false;
}

void PlatformWindow::GetErrorMessage()
{
	/*LPVOID lpMsgBuf;
	DWORD ErrorCode = GetLastError();

	DWORD WINAPI FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL);*/
	//TODO: Implementation
}

bool PlatformWindow::PrepareForRender()
{	
	MSG msg = { 0 };
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
	{
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

void PlatformWindow::PostRender()
{
	SwapBuffers(DeviceContext);
}

LRESULT PlatformWindow::WindowProcessMessages(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_CREATE:
		{
			PIXELFORMATDESCRIPTOR PreferredPixelFormat =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,	//Flags
				PFD_TYPE_RGBA,												//The kind of framebuffer. RGBA or palette.
				32,															//Colordepth of the framebuffer.
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				24,															//Number of bits for the depthbuffer
				8,															//Number of bits for the stencilbuffer
				0,															//Number of Aux buffers in the framebuffer.
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			GlobalWindow->DeviceContext = GetDC(Window);
			int RequestedPixelFormat = ChoosePixelFormat(GlobalWindow->DeviceContext, &PreferredPixelFormat);
			SetPixelFormat(GlobalWindow->DeviceContext, RequestedPixelFormat, &PreferredPixelFormat);

			GlobalWindow->GLRenderContext = wglCreateContext(GlobalWindow->DeviceContext);
			if (wglMakeCurrent(GlobalWindow->DeviceContext, GlobalWindow->GLRenderContext))
			{
				//MessageBoxA(0, (char*) glGetString(GL_VERSION), "OPENGL VERSION", 0);
			}

			glewInit();

			if (WGLEW_EXT_swap_control && !GlobalWindow->WindowParams.UseVSync)
			{
				wglSwapIntervalEXT(0);
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

			SetCapture(Window);
			ShowCursor(FALSE);

			break;
		}
		case WM_SIZE:
		{
			// Height is in the high-order of lParam, Width is in the low-order
			unsigned int NewWidth = LOWORD(lParam);
			unsigned int NewHeight = HIWORD(lParam);
			glViewport(0, 0, NewWidth, NewHeight);
			break;
		}
		case WM_DESTROY:
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(GlobalWindow->GLRenderContext);
			PostQuitMessage(0);

			UnloadFontLibrary();

			break;
		}
		case WM_KEYDOWN:
		{
			Input::Keys[wParam] = true;

			if (wParam == VK_ESCAPE)
			{
				DestroyWindow(Window);
			}
			break;
		}
		case WM_KEYUP:
		{
			Input::Keys[wParam] = false;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			Input::Keys[wParam] = true;
			break;
		}
		case WM_MBUTTONUP:
		{
			Input::Keys[wParam] = false;
			break;
		}
		case WM_MOUSEMOVE:
		{
			Input::MouseX = GET_X_LPARAM(lParam);
			Input::MouseY = GET_Y_LPARAM(lParam);
			break;
		}
		default:
		{
			return DefWindowProc(Window, Message, wParam, lParam);
		}
	}
	return 0;
}
