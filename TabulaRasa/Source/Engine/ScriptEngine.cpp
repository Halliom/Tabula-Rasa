#include "ScriptEngine.h"

#include <string>

#include "Console.h"
#include "../Game/World.h"
#include "../Platform/Platform.h"

extern Console* g_Console;
extern World* g_World;

lua_State* Script::g_State = NULL;

void LogToConsole(std::string Message)
{
	g_Console->PrintLine(Message);
}

void WorldWrapper::AddBlock(int X, int Y, int Z, int BlockID)
{
	g_World->AddBlock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveBlock(int X, int Y, int Z)
{
	g_World->RemoveBlock(X, Y, Z);
}

void WorldWrapper::AddMultiblock(int X, int Y, int Z, int BlockID)
{
	g_World->AddMultiblock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveMultiblock(int X, int Y, int Z)
{
	g_World->RemoveMultiblock(X, Y, Z);
}

Script::Script(char* Filename)
{
	char* ScriptSource = PlatformFileSystem::LoadScript(Filename);
	
	if (!g_State)
	{
		g_State = luaL_newstate();
		luaL_openlibs(g_State);

		luabridge::getGlobalNamespace(g_State)
			.addFunction("print", &LogToConsole)
			.beginNamespace("world")
				.addFunction("add_block", &WorldWrapper::AddBlock)
				.addFunction("remove_block", &WorldWrapper::RemoveBlock)
				.addFunction("add_multiblock", &WorldWrapper::AddMultiblock)
				.addFunction("remove_mutliblock", &WorldWrapper::RemoveMultiblock)
			.endNamespace();
	}

	int Error = luaL_dostring(g_State, ScriptSource);
	if (Error != 0)
	{
		LogToConsole("Failed to load script");
	}
}

void Script::CallMethod(char* MethodName)
{
	luabridge::LuaRef Method = luabridge::getGlobal(g_State, MethodName);
	Method();
}