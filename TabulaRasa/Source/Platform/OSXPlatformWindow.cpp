#ifdef __APPLE__
#include "OSXPlatformWindow.h"

#include "glm/common.hpp"

#include "../Engine/Input.h"
#include "../Engine/Camera.h"
#include "../Engine/Engine.h"

#include "../Rendering/Fonts.h"
#include "../Rendering/RenderingEngine.h"

#include "../Engine/Console.h"

#include "../Rendering/GUI/imgui/imgui.h"

PlatformWindow* PlatformWindow::GlobalWindow = NULL;

PlatformWindow::PlatformWindow(const WindowParameters& WindowParams) :
WindowParams(WindowParams)
{
    GlobalWindow = this;
}

PlatformWindow::~PlatformWindow()
{
    GlobalWindow = NULL;
}

bool PlatformWindow::SetupWindowAndRenderContext()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        //TODO: Print to log since rendering isn't initialized yet
        return false;
    }
    
    uint32_t Flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_ALLOW_HIGHDPI ;
    
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
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    MainWindow = SDL_CreateWindow(
                                  WindowParams.Title,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WindowParams.Width,
                                  WindowParams.Height,
                                  Flags);
    if (MainWindow == NULL)
    {
        //TODO: Print to log since rendering isn't initialized yet
        return false;
    }
    
    SDL_StartTextInput();
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    // Initializes the input for the mouse
    int MouseX = 0, MouseY = 0;
    SDL_GetMouseState(&MouseX, &MouseY);
    Input::MouseX = MouseX;
    Input::MouseY = MouseY;
    
    // Create the OpenGL context to draw to
    MainContext = SDL_GL_CreateContext(MainWindow);
    
    SDL_GL_SetSwapInterval(1);
    
    // Make sure that OpenGL works
    if (gl3wInit() != 0 || !gl3wIsSupported(3, 2))
    {
        // TODO: Exit/crash with error
        assert(false);
    }
    
    if (GlobalWindow->WindowParams.UseVSync)
    {
        SDL_GL_SetSwapInterval(1);
    }
    else
    {
        SDL_GL_SetSwapInterval(0);
    }
    
    int DrawWidth, DrawHeight;
    SDL_GL_GetDrawableSize(MainWindow, &DrawWidth, &DrawHeight);
    WindowParams.Width = DrawWidth;
    WindowParams.Height = DrawHeight;
    
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
    Input::MouseWheel = 0.0f;
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
                        if (Camera::g_ActiveCamera)
                        {
                            int Width, Height;
                            SDL_GL_GetDrawableSize(MainWindow, &Width, &Height);
                            WindowParams.Width = Width;
                            WindowParams.Height = Height;
                            
                            Camera::g_ActiveCamera->m_WindowWidth = WindowParams.Width;
                            Camera::g_ActiveCamera->m_WindowHeight = WindowParams.Height;
                            Camera::g_ActiveCamera->m_bIsScreenMatrixDirty = true;
                            Camera::g_ActiveCamera->m_bIsProjectionMatrixDirty = true;
                            
                            g_Engine->g_RenderingEngine->ScreenDimensionsChanged(WindowParams.Width, WindowParams.Height);
                            g_Engine->g_GUIRenderer->UpdateScreenDimensions(WindowParams.Width, WindowParams.Height);
                        }
                        break;
                    }
                }
                break;
            }
            case SDL_TEXTINPUT:
            {
                ImGuiIO& IO = ImGui::GetIO();
                IO.AddInputCharactersUTF8(Event.text.text);
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                if (Event.wheel.y > 0)
                    Input::MouseWheel = 1.0f;
                else if (Event.wheel.y < 0)
                    Input::MouseWheel = -1.0f;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (Event.key.keysym.scancode >= 512)
                    continue;
                switch (Event.key.keysym.scancode)
                {
#ifdef _DEBUG
                    case SDL_SCANCODE_ESCAPE:
                    {
                        if (g_Engine->g_Console->m_bShowConsole)
                        {
                            g_Engine->g_Console->ShowConsole(false);
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    }
                    case SDL_SCANCODE_F11:
                    {
                        WindowParams.Fullscreen = !WindowParams.Fullscreen;
                        if (WindowParams.Fullscreen)
                            SDL_SetWindowFullscreen(MainWindow, SDL_WINDOW_FULLSCREEN);
                        else
                            SDL_SetWindowFullscreen(MainWindow, 0);
                        break;
                    }
                    case SDL_SCANCODE_F1:
                    {
                        g_Engine->g_Console->ShowConsole(!g_Engine->g_Console->m_bShowConsole);
                        break;
                    }
                    case SDL_SCANCODE_F10:
                    {
                        SDL_MaximizeWindow(MainWindow);
                        break;
                    }
#endif
                        // TODO: Remove in build perhaps?
                    case SDL_SCANCODE_GRAVE:
                    {
                        bool NewMode = !(bool)SDL_GetRelativeMouseMode();
                        Input::IsGameFrozen = !NewMode;
                        SDL_SetRelativeMouseMode((SDL_bool)NewMode);
                        continue;
                    }
                }
                
                int Key = Event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
                ImGuiIO& IO = ImGui::GetIO();
                IO.KeysDown[Key] = true;
                IO.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
                IO.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
                IO.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
                IO.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
                
                if (!Input::IsGameFrozen)
                {
                    Input::Keys[Event.key.keysym.scancode] = true;
                }
                break;
            }
            case SDL_KEYUP:
            {
                int Key = Event.key.keysym.sym & ~SDLK_SCANCODE_MASK;
                ImGuiIO& IO = ImGui::GetIO();
                IO.KeysDown[Key] = false;
                
                if (!Input::IsGameFrozen)
                {
                    Input::Keys[Event.key.keysym.scancode] = false;
                }
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
    return true;
}

void PlatformWindow::PostRender()
{
    SDL_GL_SwapWindow(MainWindow);
}

SDL_Window* PlatformWindow::GetWindow()
{
    assert(MainWindow != NULL);
    
    return MainWindow;
}
#endif