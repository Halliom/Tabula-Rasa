#include "Platform.h"

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

void ErrorCallback(int Error, const char* Description)
{
    return;
}

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods)
{
    if (Action == GLFW_PRESS)
        Input::MouseButtons[Button] = true;
    else
        Input::MouseButtons[Button] = false;
}

void MouseCallback(GLFWwindow* Window, double PosX, double PosY)
{
    Input::MouseX = PosX;
    Input::MouseY = PosY;
}

void ScrollCallback(GLFWwindow* Window, double DeltaX, double DeltaY)
{
    Input::MouseWheel += DeltaY;
}

void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
    ImGuiIO& IO = ImGui::GetIO();
    if (Action == GLFW_PRESS)
    {
        IO.KeysDown[Key] = true;
        if (!Input::IsGameFrozen)
            Input::Keys[Key] = true;
        
#ifdef _DEBUG
        if (Key == GLFW_KEY_ESCAPE)
        {
            if (g_Engine->g_Console->m_bShowConsole)
            {
                g_Engine->g_Console->ShowConsole(false);
            }
            else
            {
                // Otherwise we want to exit the application
                glfwSetWindowShouldClose(Window, true);
            }
        }
        if (Key == GLFW_KEY_F11)
        {
            // TODO: Toggle fullscreen
        }
        if (Key == GLFW_KEY_F1)
        {
            g_Engine->g_Console->ShowConsole(!g_Engine->g_Console->m_bShowConsole);
        }
        if (Key == GLFW_KEY_GRAVE_ACCENT)
        {
            if (glfwGetInputMode(Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            {
                glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                Input::IsGameFrozen = true;
            }
            else
            {
                glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                Input::IsGameFrozen = false;
            }
        }
#endif
    }
    if (Action == GLFW_RELEASE)
    {
        IO.KeysDown[Key] = false;
        if (!Input::IsGameFrozen)
            Input::Keys[Key] = false;
    }
    
    IO.KeyCtrl = IO.KeysDown[GLFW_KEY_LEFT_CONTROL] || IO.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    IO.KeyShift = IO.KeysDown[GLFW_KEY_LEFT_SHIFT] || IO.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    IO.KeyAlt = IO.KeysDown[GLFW_KEY_LEFT_ALT] || IO.KeysDown[GLFW_KEY_RIGHT_ALT];
    IO.KeySuper = IO.KeysDown[GLFW_KEY_LEFT_SUPER] || IO.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void CharCallback(GLFWwindow* Window, unsigned int Char)
{
    ImGuiIO& IO = ImGui::GetIO();
    if (Char > 0 && Char < 0x10000)
        IO.AddInputCharacter((unsigned short)Char);
}

void WindowCallback(GLFWwindow* Window, int Width, int Height)
{
    PlatformWindow* GlobalWindow = PlatformWindow::GlobalWindow;
    if (Camera::g_ActiveCamera)
    {
        GlobalWindow->WindowParams.Width = Width;
        GlobalWindow->WindowParams.Height = Height;
        
        Camera::g_ActiveCamera->UpdateScreenDimensions(Width, Height);
        g_Engine->g_RenderingEngine->UpdateScreenDimensions(Width, Height);
        g_Engine->g_GUIRenderer->UpdateScreenDimensions(Width, Height);
    }
}

bool PlatformWindow::SetupWindowAndRenderContext()
{
    if (!glfwInit())
    {
        //TODO: Print to log since rendering isn't initialized yet
        glfwTerminate();
        return false;
    }
    
    glfwSetErrorCallback(&ErrorCallback);
    
    GLFWmonitor* Monitor = NULL;
    if (WindowParams.Fullscreen)
    {
        Monitor = glfwGetPrimaryMonitor();
    }
    if (WindowParams.StartMaximized)
    {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    
    //glfwWindowHint(GLFW_RED_BITS, 8);
    //glfwWindowHint(GLFW_GREEN_BITS, 8);
    //glfwWindowHint(GLFW_BLUE_BITS, 8);
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    
    MainWindow = glfwCreateWindow(WindowParams.Width, WindowParams.Height, WindowParams.Title, Monitor, NULL);
    if (!MainWindow)
    {
        //TODO: Print to log since rendering isn't initialized yet
        assert(false);
        return false;
    }
    
    glfwSetInputMode(MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    int Width, Height;
    glfwGetFramebufferSize(MainWindow, &Width, &Height);
    WindowParams.Width = Width;
    WindowParams.Height = Height;
    
    // Create the OpenGL context to draw to
    glfwMakeContextCurrent(MainWindow);
    
    glewExperimental = true;
    if (glewInit())
    {
        assert(false);
        return false;
    }
    
    // Set up the input callbacks
    glfwSetMouseButtonCallback(MainWindow, &MouseButtonCallback);
    glfwSetCursorPosCallback(MainWindow, &MouseCallback);
    glfwSetScrollCallback(MainWindow, &ScrollCallback);
    glfwSetKeyCallback(MainWindow, &KeyCallback);
    glfwSetCharCallback(MainWindow, &CharCallback);
    glfwSetWindowSizeCallback(MainWindow, &WindowCallback);
    
    if (GlobalWindow->WindowParams.UseVSync)
    {
        glfwSwapInterval(1);
    }
    else
    {
        glfwSwapInterval(0);
    }
    
    return true;
}

void PlatformWindow::DestroyWindow()
{
    glfwDestroyWindow(MainWindow);
    
    glfwTerminate();
}

void PlatformWindow::GetErrorMessage()
{
}

bool PlatformWindow::PrepareForRender()
{
    Input::MouseWheel = 0.0f;
    glfwPollEvents();
    return !glfwWindowShouldClose(MainWindow);
}

void PlatformWindow::PostRender()
{
    glfwSwapBuffers(MainWindow);
}

GLFWwindow* PlatformWindow::GetWindow()
{
    assert(MainWindow != NULL);

    return MainWindow;
}
