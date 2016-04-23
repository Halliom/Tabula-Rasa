#include "ScriptEngine.h"

#include "Console.h"

extern Console* g_Console;

#include "lua5.2\lua.hpp"

#include "LuaBridge/LuaBridge.h"

void luaprint()
{
	g_Console->PrintLine("Hello from Lua");
}

void DoSomething()
{
	lua_State* State = luaL_newstate();
	luaL_openlibs(State);

	luabridge::getGlobalNamespace(State).
		addFunction("log", &luaprint);

	luaL_dostring(State, "log()");
}