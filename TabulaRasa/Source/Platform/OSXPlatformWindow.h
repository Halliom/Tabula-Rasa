#pragma once

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#include "SDL2/SDL.h"
#elif __APPLE__
#include <OpenGL/gl3.h>
#include "SDL2OSX/SDL.h"
#endif

struct WindowParameters
{
    const char* Title;
    unsigned int Width;
    unsigned int Height;
    bool UseVSync;
    bool UseDepthTest;
    bool Fullscreen;
    bool StartMaximized;
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
        
    WindowParameters WindowParams;
    
    static PlatformWindow* GlobalWindow;
    
private:
    
    SDL_Window* MainWindow;
    
    SDL_GLContext MainContext;
};