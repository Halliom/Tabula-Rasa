#include "ScriptEngine.h"

#include <string>

#include "lua5.2\lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "Console.h"

extern Console* g_Console;

void luaprint(std::string Message)
{
	g_Console->PrintLine(Message);
}

void DoSomething()
{
	lua_State* State = luaL_newstate();
	luaL_openlibs(State);

	luabridge::getGlobalNamespace(State).
		addFunction("print", &luaprint);

	luaL_dostring(State, "print(\"Hello World\")");
}