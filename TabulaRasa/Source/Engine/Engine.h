#pragma once

#ifdef _WIN32
#define FORCEINLINE __forceinline
#elif __APPLE__
#define FORCEINLINE inline
#endif

#include <assert.h>

#define ArrayCount(Array) sizeof(Array) / sizeof(Array[0])

#define Max(x, y) (((x) > (y)) ? (x) : (y))
#define Min(x, y) (((x) < (y)) ? (x) : (y))

struct EngineGlobals
{
    class GameMemoryManager*    g_MemoryManager;
    class ScriptEngine*         g_ScriptEngine;
    class FontLibrary*          g_FontLibrary;
    class Console*              g_Console;
    class Renderer*				g_WorldRenderer;
    class DebugGUIRenderer*     g_GUIRenderer;
    class World*                g_World;
};

extern EngineGlobals* g_Engine;