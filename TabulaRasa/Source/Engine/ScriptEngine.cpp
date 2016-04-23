#include "ScriptEngine.h"

#include <string>

#include "Console.h"

#include "../Platform/Platform.h"

extern Console* g_Console;

lua_State* Script::g_State = NULL;

void LogToConsole(std::string Message)
{
	g_Console->PrintLine(Message);
}

Script::Script(char* Filename)
{
	char* ScriptSource = PlatformFileSystem::LoadScript(Filename);
	
	if (!g_State)
	{
		g_State = luaL_newstate();
		luaL_openlibs(g_State);

		luabridge::getGlobalNamespace(g_State)
			.addFunction("print", &LogToConsole);
	}

	int Error = luaL_dostring(g_State, ScriptSource);
	if (Error != 0)
	{
		LogToConsole("Failed to load script: ");
	}
}

void Script::CallMethod(char* MethodName)
{
	luabridge::LuaRef Method = luabridge::getGlobal(g_State, MethodName);
	Method();
}