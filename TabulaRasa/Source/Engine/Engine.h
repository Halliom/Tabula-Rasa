#pragma once

#ifdef _WIN32
#define FORCEINLINE __forceinline
#elif __APPLE__
#define FORCEINLINE inline
#endif

struct EngineGlobals
{
    class GameMemoryManager*    g_MemoryManager;
    class Console*              g_Console;
    class RenderingEngine*      g_RenderingEngine;
    class DebugGUIRenderer*     g_GUIRenderer;
    class World*                g_World;
};

extern EngineGlobals* g_Engine;